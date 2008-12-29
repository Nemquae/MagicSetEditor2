
//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/spell_checker.hpp>
#include <util/tagged_string.hpp>

// ----------------------------------------------------------------------------- : Functions

bool spelled_correctly(const String& input, size_t start, size_t end, SpellChecker& checker) {
	// untag
	String word = untag(input.substr(start,end-start));
	if (word.empty()) return true;
	// remove punctuation
	size_t start_u = 0, end_u = String::npos;
	trim_punctuation(word, start_u, end_u);
	if (start_u >= end_u) {
		// punctuation only, but not empty => error
		return false;
	}
	// find the tagged text without punctuation
	size_t start2 = untagged_to_index(input, start_u, true, start);
	size_t end2   = untagged_to_index(input, end_u,   true, start);
	if (in_tag(input,_("<sym"),start2,end2) != String::npos) {
		// symbols are always spelled correctly
		return true;
	}
	// run through spellchecker
	return checker.spell(word.substr(start_u,end_u));
}

void check_word(const String& input, String& out, size_t start, size_t end, SpellChecker& checker) {
	if (start >= end) return;
	bool good = spelled_correctly(input, start, end, checker);
	if (!good) out += _("<error-spelling>");
	out.append(input, start, end-start);
	if (!good) out += _("</error-spelling>");
}

SCRIPT_FUNCTION(check_spelling) {
	SCRIPT_PARAM(String,language);
	SCRIPT_PARAM(String,input);
	if (language.empty()) {
		// no language -> spelling checking
		SCRIPT_RETURN(true);
	}
	SpellChecker& checker = SpellChecker::get(language);
	// remove old spelling error tags
	input = remove_tag(input, _("<error-spelling"));
	// now walk over the words in the input, and mark misspellings
	String result;
	size_t word_start = 0, word_end = 0, pos = 0;
	while (pos < input.size()) {
		Char c = input.GetChar(pos);
		if (c == _('<')) {
			if (word_start == pos) {
				// prefer to place word start inside tags
				pos = skip_tag(input,pos);
				result.append(input, word_start, pos - word_start);
				word_end = word_start = pos;
			} else {
				pos = skip_tag(input,pos);
			}
		} else if (isSpace(c) || c == EM_DASH || c == EN_DASH) {
			// word boundary -> check word
			check_word(input, result, word_start, word_end, checker);
			// non-word characters
			result.append(input, word_end, pos - word_end + 1);
			// next
			word_start = word_end = pos = pos + 1;
		} else {
			word_end = pos = pos + 1;
		}
	}
	// last word
	check_word(input, result, word_start, word_end, checker);
	result.append(input, word_end, String::npos);
	// done
	SCRIPT_RETURN(result);
}

SCRIPT_FUNCTION(check_spelling_word) {
	SCRIPT_PARAM(String,language);
	SCRIPT_PARAM(String,input);
	if (language.empty()) {
		// no language -> spelling checking
		SCRIPT_RETURN(true);
	} else {
		bool correct = SpellChecker::get(language).spell(input);
		SCRIPT_RETURN(correct);
	}
}

// ----------------------------------------------------------------------------- : Init

void init_script_spelling_functions(Context& ctx) {
	ctx.setVariable(_("check spelling"),       script_check_spelling);
	ctx.setVariable(_("check spelling word"),  script_check_spelling_word);
}
