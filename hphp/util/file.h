/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_UTIL_FILE_H_
#define incl_HPHP_UTIL_FILE_H_

#include <folly/Range.h>

#include <ctype.h>

namespace HPHP { namespace FileUtil {
////////////////////////////////////////////////////////////////////////////////

/*
 * Check if the given character is a directory separator for the current
 * platform.
 */
inline bool isDirSeparator(char c) {
#ifdef _MSC_VER
  return c == '/' || c == '\\';
#else
  return c == '/';
#endif
}

/*
 * Get the preferred directory separator for the current platform.
 */
inline char getDirSeparator() {
#ifdef _MSC_VER
  return '\\';
#else
  return '/';
#endif
}

/*
 * Check if the given path is an absolute path.  This does not guarantee that
 * the path is canonicalized.
 */
inline bool isAbsolutePath(folly::StringPiece path) {
#ifdef _MSC_VER
  auto const len = path.size();
  if (len < 2) {
    return false;
  }
  // NOTE: Boost actually checks if the last character of the first path element
  // is a colon, rather than if the first character is an alpha followed by a
  // colon.  This is fine for now, as I don't know of any other forms of paths
  // that would allow.
  return (isDirSeparator(path[0]) && isDirSeparator(path[1])) ||
    (isalpha(path[0]) && path[1] == ':');
#else
  return path.size() > 0 && path[0] == '/';
#endif
}

////////////////////////////////////////////////////////////////////////////////
}}

#endif
