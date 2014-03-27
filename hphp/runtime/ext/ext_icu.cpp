/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/ext_icu.h"
#include <vector>
#include <string>
#include <boost/scoped_ptr.hpp>
#include <unicode/rbbi.h>
#include <unicode/translit.h>
#include <unicode/uregex.h>
#include <unicode/ustring.h>
#include "icu/LifeEventTokenizer.h" // nolint
#include "icu/ICUMatcher.h" // nolint
#include "icu/ICUTransliterator.h" // nolint

using namespace U_ICU_NAMESPACE;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
const int64_t k_UREGEX_CASE_INSENSITIVE = UREGEX_CASE_INSENSITIVE;
const int64_t k_UREGEX_COMMENTS         = UREGEX_COMMENTS;
const int64_t k_UREGEX_DOTALL           = UREGEX_DOTALL;
const int64_t k_UREGEX_MULTILINE        = UREGEX_MULTILINE;
const int64_t k_UREGEX_UWORD            = UREGEX_UWORD;
// Intentionally higher in case ICU adds more constants.
const int64_t k_UREGEX_OFFSET_CAPTURE   = 1LL<<32;

///////////////////////////////////////////////////////////////////////////////
typedef tbb::concurrent_hash_map<const StringData*,const RegexPattern*,
                                StringDataHashCompare> PatternStringMap;

static PatternStringMap s_patternCacheMap;

Variant f_icu_match(const String& pattern, const String& subject,
                    VRefParam matches /* = null */, int64_t flags /* = 0 */) {
  UErrorCode status = U_ZERO_ERROR;

  Array matchesArr;
  if (matches.isReferenced()) {
    matchesArr = Array::Create();
  }
  SCOPE_EXIT { if (matches.isReferenced()) matches = matchesArr; };

  // Create hash map key by concatenating pattern and flags.
  StringBuffer bpattern;
  bpattern.append(pattern);
  bpattern.append(':');
  bpattern.append(flags);
  String spattern = bpattern.detach();

  // Find compiled pattern matcher in hash map or add it.
  PatternStringMap::accessor accessor;
  const RegexPattern* rpattern;
  if (s_patternCacheMap.find(accessor, spattern.get())) {
    rpattern = accessor->second;
  } else {
    // First 32 bits are reserved for ICU-specific flags.
    rpattern = RegexPattern::compile(
      UnicodeString::fromUTF8(pattern.data()), (flags & 0xFFFFFFFF), status);
    if (U_FAILURE(status)) {
      return false;
    }

    if (s_patternCacheMap.insert(
      accessor, makeStaticString(spattern.get()))) {
      accessor->second = rpattern;
    } else {
      delete rpattern;
      rpattern = accessor->second;
    }
  }

  // Build regex matcher from compiled pattern and passed-in subject.
  UnicodeString usubject = UnicodeString::fromUTF8(subject.data());
  boost::scoped_ptr<RegexMatcher> matcher(rpattern->matcher(usubject, status));
  if (U_FAILURE(status)) {
    return false;
  }

  // Return 0 or 1 depending on whether or not a match was found and
  // (optionally), set matched (sub-)patterns for passed-in reference.
  int matched = 0;
  if (matcher->find()) {
    matched = 1;

    if (matches.isReferenced()) {
      int32_t count = matcher->groupCount();

      for (int32_t i = 0; i <= count; i++) {
        UnicodeString ustring = matcher->group(i, status);
        if (U_FAILURE(status)) {
          return false;
        }

        // Convert UnicodeString back to UTF-8.
        std::string string;
        ustring.toUTF8String(string);
        String match = String(string);

        if (flags & k_UREGEX_OFFSET_CAPTURE) {
          // start() returns the index in UnicodeString, which
          // normally means the index into an array of 16-bit
          // code "units" (not "points").
          int32_t start = matcher->start(i, status);
          if (U_FAILURE(status)) {
            return false;
          }

          start = usubject.countChar32(0, start);
          matchesArr.append(make_packed_array(match, start));
        } else {
          matchesArr.append(match);
        }
      }
    }
  }

  return matched;
}


// Need to have a valid installation of the transliteration data in /lib64.
// Initialization will be taken care of by ext_array which also uses icu.

class TransliteratorWrapper {
public:
  TransliteratorWrapper() {
    UnicodeString basicID("Any-Latin ; NFKD; [:nonspacing mark:] Remove");
    UnicodeString basicIDAccent("Any-Latin ; NFKC");
    UErrorCode status = U_ZERO_ERROR;
    m_tl = Transliterator::createInstance(basicID, UTRANS_FORWARD, status);
    // Note that if the first createInstance fails, the status will cause the
    // second createInstance to also fail.
    m_tl_accent =
      Transliterator::createInstance(basicIDAccent, UTRANS_FORWARD, status);

    if (U_FAILURE(status)) {
      raise_warning(std::string(u_errorName(status)));
      //m_tl should be NULL if createInstance fails but better safe than sorry.
      m_tl = NULL;
      m_tl_accent = NULL;
    }
  }

  void transliterate(UnicodeString& u_str) {
    if (m_tl) {
      m_tl->transliterate(u_str);
    } else {
      raise_warning("Transliterator not initialized.");
    }
  }

  void transliterate_with_accents(UnicodeString& u_str) {
    if (m_tl_accent) {
      m_tl_accent->transliterate(u_str);
    } else {
      raise_warning("Transliterator not initialized.");
    }
  }

private:
  Transliterator* m_tl;
  Transliterator* m_tl_accent;
};

IMPLEMENT_THREAD_LOCAL(TransliteratorWrapper, s_transliterator);

String f_icu_transliterate(const String& str, bool remove_accents) {
  UnicodeString u_str = UnicodeString::fromUTF8(str.data());
  if (remove_accents) {
    s_transliterator->transliterate(u_str);
  } else {
    s_transliterator->transliterate_with_accents(u_str);
  }

  // Convert UnicodeString back to UTF-8.
  std::string string;
  u_str.toUTF8String(string);
  return String(string);
}


// There are quicker ways to do this conversion, but it's necessary to follow
// this to match the functionality of fbcode/multifeed/text/TokenizeTextMap.cpp.
std::string icuStringToUTF8(const UnicodeString& ustr) {
  UErrorCode status = U_ZERO_ERROR;
  int32_t bufSize = 0;
  std::string result;

  // Calculate the size of the buffer needed to hold ustr, converted to UTF-8.
  u_strToUTF8(NULL, 0, &bufSize, ustr.getBuffer(), ustr.length(), &status);
  if (status != U_BUFFER_OVERFLOW_ERROR &&
      status != U_STRING_NOT_TERMINATED_WARNING) {
    return result;
  }

  result.resize(bufSize);

  status = U_ZERO_ERROR;
  u_strToUTF8(&result[0], bufSize, NULL, ustr.getBuffer(), ustr.length(),
              &status);

  if (U_FAILURE(status)) {
    result.clear();
  }

  return result;
}


// Regex matchers for spaces and numbers.
class SpaceMatcher : public ICUMatcher {
public:
  SpaceMatcher() { set("^\\s+$"); }
};

class NumMatcher : public ICUMatcher {
public:
  NumMatcher() { set("\\d"); }
};


// Transliterator to convert UnicodeStrings to lower case.
class LowerCaseTransliterator : public ICUTransliterator {
public:
  LowerCaseTransliterator() { set("Upper; Lower;"); }
};


// Thread-local globals.
IMPLEMENT_THREAD_LOCAL(SpaceMatcher, s_spaceMatcher);
IMPLEMENT_THREAD_LOCAL(NumMatcher, s_numMatcher);
IMPLEMENT_THREAD_LOCAL(LowerCaseTransliterator, s_lctranslit);


/* Normalize a unicode string depending on its type.
 * See icu/Tokenizer.cpp for definition of types.
 */
void normalizeToken(struct Token& token) {
  UnicodeString& str = token.value;
  int32_t type = token.status;

  switch (type) {
    // punctuations
    case 0: break;
    case 100: str = s_numMatcher->replaceAll(str, "X"); break;
    // words
    case 200: s_lctranslit->transliterate(str); break;
    // katekana/hiragana
    case 300: s_lctranslit->transliterate(str); break;
    // ideographic
    case 400: s_lctranslit->transliterate(str); break;
    case 500: str = "TOKEN_EMAIL"; break;
    case 501: str = "TOKEN_URL";  break;
    // emoticon
    case 502: s_lctranslit->transliterate(str); break;
    case 503: str = "TOKEN_HEART"; break;
    // exclamation
    case 504: break;
    case 505: str = "TOKEN_DATE";  break;
    case 506: str = "TOKEN_MONEY"; break;
    case 507: str = "TOKEN_TIME";  break;
    //acronym, lower casing because could just be capitalized word
    case 508: s_lctranslit->transliterate(str); break;
    default: str = "";
  }
}


/* Returns a list of tokens, but with various normalizations performed
 * based on the token type.
 *
 * Default behavior:
 * Whitespace: dropped (removed from output)
 * Words: converted to lower case
 * Numbers: replaced with #XXX, where the number of X's is based on the
 *          format of the number; any punctuation is maintained
 * Japanese/Chinese scripts: converted to lower case
 * Email: Converted to TOKEN_EMAIL
 * URL: Converted to TOKEN_URL
 * Emoticon: Left as-is
 * Heart: Converted to TOKEN_HEART
 * Exclamation: Replaced with an empty string
 * Date: Replaced with TOKEN_DATE
 * Money: Replaced with TOKEN_MONEY
 * Time: Replaced with TOKEN_TIME
 * Acronym: converted to lower case
 * Other: replaced with empty string
 *
 */
Array f_icu_tokenize(const String& text) {
  // Boundary markers that indicate the beginning and end of a token stream.
  const String BEGIN_MARKER("_B_");
  const String END_MARKER("_E_");

  Array ret;
  std::vector<Token> tokens;
  tokenizeString(tokens, getMaster(), UnicodeString::fromUTF8(text.data()));

  int i = 0;
  ret.set(i++, BEGIN_MARKER);
  for(std::vector<Token>::iterator iter = tokens.begin();
      iter != tokens.end();
      iter++) {
    normalizeToken(*iter);
    const UnicodeString& word = iter->value;
    // Ignore spaces and empty strings.
    if (!s_spaceMatcher->matches(word) && word.length() > 0) {
      ret.set(i++, String(icuStringToUTF8(word)));
    }
  }
  ret.set(i++, END_MARKER);
  return ret;
}





///////////////////////////////////////////////////////////////////////////////
}
