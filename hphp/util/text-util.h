/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

// Utilities for manipulating text in strings.

#ifndef incl_HPHP_TEXT_UTIL_H_
#define incl_HPHP_TEXT_UTIL_H_

#include <string>
#include <vector>
#include "hphp/util/portability.h"

namespace HPHP {

class TextUtil {
 public:
  // Split "/foo/bar/x" into "/foo", "/foo/bar".
  // Also splits "foo/bar/x" into "foo", "foo/bar".
  static std::vector<std::string> MakePathList(const std::string& path);
};

/**
 * Split a string into a list of tokens by character delimiter.
 */
void split(char delimiter, const char *s, std::vector<std::string> &out,
           bool ignoreEmpty = false);

/**
 * Replace all occurrences of "from" substring to "to" string.
 */
void replaceAll(std::string &s, const char *from, const char *to);

/**
 * Change an ASCII string to lower case.
 */
std::string toLower(const std::string &s);

/**
 * Change an ASCII string to upper case.
 */
std::string toUpper(const std::string &s);

/**
 * Convert a full pathname of a file to an identifier.
 */
std::string getIdentifier(const std::string &fileName);

/**
 * Duplicate a buffer of given size, null-terminate the result.
 */
const void *buffer_duplicate(const void *src, int size);

/**
 * Append buf2 to buf2, null-terminate the result.
 */
const void *buffer_append(const void *buf1, int size1,
                          const void *buf2, int size2);

/**
 * printf into a std::string.
 */
void string_printf(std::string &msg,
                   const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);

/**
 * Escaping strings for code generation.
 */
std::string escapeStringForCPP(const char *input, int len,
                               bool* binary = nullptr);

inline std::string escapeStringForCPP(const std::string &input,
                                      bool* binary = nullptr) {
  return escapeStringForCPP(input.data(), input.length(), binary);
}

std::string escapeStringForPHP(const char *input, int len);

inline std::string escapeStringForPHP(const std::string &input) {
  return escapeStringForPHP(input.data(), input.length());
}

/**
 * Format a regex pattern by surrounding with slashes and escaping pattern.
 */
std::string format_pattern(const std::string &pattern, bool prefixSlash);

}  // namespace HPHP

#endif  // incl_HPHP_TEXT_UTIL_H_
