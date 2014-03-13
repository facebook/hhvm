/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_ZEND_STRING_H_
#define incl_HPHP_ZEND_STRING_H_

#include "hphp/zend/zend-string.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/string-buffer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
/**
 * Low-level string functions PHP uses.
 *
 * 1. If a function returns a char *, it has malloc-ed a new string and it's
 *    caller's responsibility to free it.
 *
 * 2. If a function takes "int &len" right after the 1st string parameter, it
 *    is input string's length, and in return, it's return string's length.
 *
 * 3. All functions work with binary strings and all returned strings are
 *    NULL terminated, regardless of whether it's a binary string.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
int string_copy(char *dst, const char *src, int siz);

/**
 * Compare two binary strings.
 */
inline int string_strcmp(const char *s1, int len1, const char *s2, int len2) {
  int minlen = len1 < len2 ? len1 : len2;
  int retval;

  retval = memcmp(s1, s2, minlen);
  if (!retval) {
    return (len1 - len2);
  }

  return (retval > 0) - (retval < 0);
}
/**
 * Compare two binary strings of the first n bytes.
 */
inline int string_strncmp(const char *s1, int len1, const char *s2, int len2,
                          int len) {
  int minlen = len1 < len2 ? len1 : len2;
  int retval;

  if (len < minlen) {
    if (UNLIKELY(len < 0)) len = 0;
    minlen = len;
  }
  retval = memcmp(s1, s2, minlen);
  if (!retval) {
    return (len < len1 ? len : len1) - (len < len2 ? len : len2);
  } else {
    return retval;
  }
}
/**
 * Compare two binary strings of the first n bytes, ignore case.
 */
inline int string_strncasecmp(const char *s1, int len1,
                              const char *s2, int len2, int len) {
  int minlen = len1 < len2 ? len1 : len2;
  int c1, c2;

  if (len < minlen) {
    if (UNLIKELY(len < 0)) len = 0;
    minlen = len;
  }
  while (minlen--) {
    c1 = tolower((int)*(unsigned char *)s1++);
    c2 = tolower((int)*(unsigned char *)s2++);
    if (c1 != c2) {
      return c1 - c2;
    }
  }
  return (len < len1 ? len : len1) - (len < len2 ? len : len2);
}

/**
 * Compare strings.
 */
int string_ncmp(const char *s1, const char *s2, int len);
int string_natural_cmp(char const *a, size_t a_len,
                       char const *b, size_t b_len, int fold_case);

/**
 * Changing string's cases. Return's length is always the same as "len".
 */
char *string_to_case(const char *s, int len, int (*tocase)(int));
char *string_to_case_first(const char *s, int len, int (*tocase)(int));
char *string_to_case_words(const char *s, int len, int (*tocase)(int));

// Use lambdas wrapping the ctype.h functions because of linker weirdness on
// OS X Mavericks.

#define string_to_upper(s,len)        \
  string_to_case((s), (len), [] (int i) -> int { return toupper(i); })
#define string_to_upper_first(s, len) \
  string_to_case_first((s), (len), [] (int i) -> int { return toupper(i); })
#define string_to_upper_words(s, len) \
  string_to_case_words((s), (len), [] (int i) -> int { return toupper(i); })

#define string_to_lower(s,len)        \
  string_to_case((s), (len), [] (int i) -> int { return tolower(i); })
#define string_to_lower_first(s, len) \
  string_to_case_first((s), (len), [] (int i) -> int { return tolower(i); })
#define string_to_lower_words(s, len) \
  string_to_case_words((s), (len), [] (int i) -> int { return tolower(i); })

/**
 * Pad a string with pad_string to pad_length. "len" is
 * input string's length, and in return, it's trimmed string's length. pad_type
 * can be k_STR_PAD_RIGHT, k_STR_PAD_LEFT or k_STR_PAD_BOTH.
 */
char *string_pad(const char *input, int &len, int pad_length,
                 const char *pad_string, int pad_str_len, int pad_type);

/**
 * Get a substring of input from "start" position with specified length.
 * "start" and "length" can be negative and refer to PHP's doc for meanings.
 */
char *string_substr(const char *s, int &len, int start, int length,
                    bool nullable);

/**
 * Find a character or substring and return it's position (or -1 if not found).
 */
int string_find(const char *input, int len, char ch, int pos,
                bool case_sensitive);
int string_rfind(const char *input, int len, char ch, int pos,
                 bool case_sensitive);
int string_find(const char *input, int len, const char *s, int s_len,
                int pos, bool case_sensitive);
int string_rfind(const char *input, int len, const char *s, int s_len,
                 int pos, bool case_sensitive);

const char *string_memnstr(const char *haystack, const char *needle,
                           int needle_len, const char *end);

/**
 * Replace specified substring or search string with specified replacement.
 */
String string_replace(const char *s, int len, int start, int length,
                      const char *replacement, int len_repl);
String string_replace(const char *input, int len,
                      const char *search, int len_search,
                      const char *replacement, int len_replace,
                      int &count, bool case_sensitive);

/**
 * Replace a substr with another and return replaced one. Note, read
 * http://www.php.net/substr about meanings of negative start or length.
 *
 * The form that takes a "count" reference will still replace all occurrences
 * and return total replaced count in the out parameter. It does NOT mean
 * it will replace at most that many occurrences, so count's input value
 * is never checked.
 */
inline String string_replace(const String& str, int start, int length,
                             const String& repl) {
  return string_replace(str.data(), str.size(), start, length,
                        repl.data(), repl.size());
}

inline String string_replace(const String& str, const String& search,
                             const String& replacement,
                             int &count, bool caseSensitive) {
  count = 0;
  if (!search.empty() && !str.empty()) {
    auto ret = string_replace(str.data(), str.size(),
                              search.data(), search.size(),
                              replacement.data(), replacement.size(),
                              count, caseSensitive);
    if (!ret.isNull()) {
      return ret;
    }
  }
  return str;
}

inline String string_replace(const String& str, const String& search,
                             const String& replacement) {
  int count;
  return string_replace(str, search, replacement, count, true);
}

/**
 * Reverse, repeat or shuffle a string.
 */
char *string_reverse(const char *s, int len);
char *string_repeat(const char *s, int &len, int count);
char *string_shuffle(const char *str, int len);
char *string_chunk_split(const char *src, int &srclen, const char *end,
                         int endlen, int chunklen);

/**
 * Strip HTML and PHP tags.
 */
char *string_strip_tags(const char *s, int &len, const char *allow,
                        int allow_len, bool allow_tag_spaces);

/**
 * Wrap text on word breaks.
 */
char *string_wordwrap(const char *text, int &textlen, int linelength,
                      const char *breakchar, int breakcharlen, bool docut);

/**
 * Encoding/decoding strings according to certain formats.
 */
char *string_addcslashes(const char *str, int &length, const char *what,
                         int wlength);
char *string_stripcslashes(const char *input, int &nlen);
char *string_addslashes(const char *str, int &length);
char *string_stripslashes(const char *input, int &l);
char *string_quotemeta(const char *input, int &len);
char *string_quoted_printable_encode(const char *input, int &len);
char *string_quoted_printable_decode(const char *input, int &len, bool is_q);
char *string_uuencode(const char *src, int src_len, int &dest_len);
char *string_uudecode(const char *src, int src_len, int &dest_len);
char *string_base64_encode(const char *input, int &len);
char *string_base64_decode(const char *input, int &len, bool strict);
char *string_escape_shell_arg(const char *str);
char *string_escape_shell_cmd(const char *str);
std::string string_cplus_escape(const char *s, int len);

/**
 * Convert between strings and numbers.
 */
inline bool string_validate_base(int base) {
  return (2 <= base && base <= 36);
}
char *string_hex2bin(const char *input, int &len);
Variant string_base_to_numeric(const char *s, int len, int base);
char *string_long_to_base(unsigned long value, int base);
char *string_numeric_to_base(const Variant& value, int base);

/**
 * Translates characters in str_from into characters in str_to one by one,
 * assuming str_from and str_to have the same length of "trlen".
 */
void string_translate(char *str, int len, const char *str_from,
                      const char *str_to, int trlen);

/**
 * Formatting.
 */
char *string_money_format(const char *format, double value);

char *string_number_format(double d, int dec, char dec_point,
                           char thousand_sep);

/**
 * Similarity and other properties of strings.
 */
int string_levenshtein(const char *s1, int l1, const char *s2, int l2,
                       int cost_ins, int cost_rep, int cost_del);
int string_similar_text(const char *t1, int len1,
                        const char *t2, int len2, float *percent);
char *string_soundex(const char *str);

char *string_metaphone(const char *input, int word_len, long max_phonemes,
                       int traditional);

/**
 * Locale strings.
 */
char *string_convert_cyrillic_string(const char *input, int length,
                                     char from, char to);
String string_convert_hebrew_string(const String& str, int max_chars_per_line,
                                    int convert_newlines);

///////////////////////////////////////////////////////////////////////////////
// helpers

/**
 * Calculates and adjusts "start" and "length" according to string's length.
 * This function determines how those two parameters are interpreted in varies
 * substr-related functions.
 *
 * The parameter strict controls whether to disallow the empty sub-string
 * after the end.
 */
bool string_substr_check(int len, int &f, int &l, bool strict = true);

/**
 * Fills a 256-byte bytemask with input. You can specify a range like 'a..z',
 * it needs to be incrementing. This function determines how "charlist"
 * parameters are interpreted in varies functions that take a list of
 * characters.
 */
void string_charmask(const char *input, int len, char *mask);

///////////////////////////////////////////////////////////////////////////////
// mac doesn't have memrchr

#if defined(__APPLE__)
 void *memrchr(const void *s, int c, size_t n);
#endif

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_STRING_H_
