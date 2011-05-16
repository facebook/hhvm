/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_ZEND_STRING_H__
#define __HPHP_ZEND_STRING_H__

#include <util/base.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/string_buffer.h>

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
 * Duplicate a binary string. Note that NULL termination is needed even for
 * a binary string, because String class only wraps such a "safe" one that can
 * work with any functions that takes a C-string.
 */
inline char *string_duplicate(const char *s, int len) {
  char *ret = (char *)malloc(len + 1);
  memcpy(ret, s, len);
  ret[len] = '\0';
  return ret;
}

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
  return retval;
}
/**
 * Compare two binary strings of the first n bytes.
 */
inline int string_strncmp(const char *s1, int len1, const char *s2, int len2,
                          int len) {
  int minlen = len1 < len2 ? len1 : len2;
  int retval;

  if (len < minlen) {
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
 * Compare two binary strings, ignore case.
 */
inline int string_strcasecmp(const char *s1, int len1,
                             const char *s2, int len2) {
  int minlen = len1 < len2 ? len1 : len2;
  int c1, c2;

  while (minlen--) {
    c1 = tolower((int)*(unsigned char *)s1++);
    c2 = tolower((int)*(unsigned char *)s2++);
    if (c1 != c2) {
      return c1 - c2;
    }
  }
  return len1 - len2;
}
/**
 * Compare two binary strings of the first n bytes, ignore case.
 */
inline int string_strncasecmp(const char *s1, int len1,
                              const char *s2, int len2, int len) {
  int minlen = len1 < len2 ? len1 : len2;
  int c1, c2;

  if (len < minlen) {
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
 * Concatenate two into one.
 */
char *string_concat(const char *s1, int len1, const char *s2, int len2,
                    int &len);

/**
 * Compare strings.
 */
int string_cmp(const char *s1, int len1, const char *s2, int len2);
int string_casecmp(const char *s1, int len1, const char *s2, int len2);
int string_ncmp(const char *s1, const char *s2, int len);
int string_ncasecmp(const char *s1, const char *s2, int len);
int string_natural_cmp(char const *a, size_t a_len,
                       char const *b, size_t b_len, int fold_case);

/**
 * Changing string's cases. Return's length is always the same as "len".
 */
char *string_to_lower(const char *s, int len);
char *string_to_upper(const char *s, int len);
char *string_to_upper_first(const char *s, int len);
char *string_to_upper_words(const char *s, int len);

/**
 * Trim a string by removing characters in the specified charlist.
 *
 *   mode 1 : trim left
 *   mode 2 : trim right
 *   mode 3 : trim left and right
 */
char *string_trim(const char *s, int &len,
                  const char *charlist, int charlistlen, int mode);

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
void *string_memrchr(const void *s, int c, size_t n);

/**
 * Replace specified substring or search string with specified replacement.
 */
char *string_replace(const char *s, int &len, int start, int length,
                     const char *replacement, int len_repl);
char *string_replace(const char *input, int &len,
                     const char *search, int len_search,
                     const char *replacement, int len_replace,
                     int &count, bool case_sensitive);

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
                        int allow_len);

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
char *string_cplus_escape(const char *s, int len);

/**
 * Convert between strings and numbers.
 */
inline bool string_validate_base(int base) {
  return (2 <= base && base <= 36);
}
char *string_bin2hex(const char *input, int &len);
char *string_hex2bin(const char *input, int &len);
Variant string_base_to_numeric(const char *s, int len, int base);
char *string_long_to_base(unsigned long value, int base);
char *string_numeric_to_base(CVarRef value, int base);

/**
 * Translates characters in str_from into characters in str_to one by one,
 * assuming str_from and str_to have the same length of "trlen".
 */
void string_translate(char *str, int len, const char *str_from,
                      const char *str_to, int trlen);

/**
 * Hashing a string.
 */
char *string_rot13(const char *input, int len);
int   string_crc32(const char *p, int len);
char *string_crypt(const char *key, const char *salt);
char *string_md5(const char *arg, int arg_len, bool raw, int &out_len);
char *string_sha1(const char *arg, int arg_len, bool raw, int &out_len);

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
char *string_convert_hebrew_string(const char *str, int &str_len,
                                   int max_chars_per_line,
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

#endif // __HPHP_ZEND_STRING_H__
