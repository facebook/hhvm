/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

class TextUtil {
 public:
  // Returns true if the entirety of str2 is at the beginning of str1.
  // The empty string is considered to be at the beginning of a string.
  static bool BeginsWith(const std::string& str1, const std::string& str2);

  // If the entirety of str2 is at the beginning of str1, then
  // return just the unique part of str1.  Otherwise return str1 unchanged.
  static std::string StripCommonStart(const std::string& str1,
                                      const std::string& str2);

  // Returns true if str's last character is ch.
  static bool EndsWith(const std::string& str, char ch);

  // Returns str with any instances of ch trimmed from the right end.
  static std::string StripTrailing(const std::string& str, char ch);

  // Split "/foo/bar/x" into "/foo", "/foo/bar".
  // Also splits "foo/bar/x" into "foo", "foo/bar".
  static std::vector<std::string> MakePathList(const std::string& path);
};

}  // namespace HPHP

#endif  // incl_HPHP_TEXT_UTIL_H_
