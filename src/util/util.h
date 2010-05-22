/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <vector>
#include <string>
#include <map>
#include <stdlib.h>

/**
 * Simple utility functions.
 */
namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

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
 * Make sure path exists. Same as "mkdir -p", but "a/b" will only make sure
 * "a/" exists, treating "b" as a file name.
 */
bool mkdir(const std::string &path, int mode = 0777);

/**
 * Make dest directory look identical to src by copying files and directories,
 * without copying identical files (so they keep the same timestamp as before).
 */
void syncdir(const std::string &dest, const std::string &src,
             bool keepSrc = false);

/**
 * Like rename(2), but takes care of cross-filesystem rename.
 */
int rename(const char *oldname, const char *newname);

/**
 * Like system(3), but automatically print errors if execution fails.
 */
int ssystem(const char *command);

/**
 * Canonicalize path to remove "..", "." and "\/", etc..
 */
std::string canonicalize(const std::string &path);
const char *canonicalize(const char *path, size_t len);
/**
 * Thread-safe strerror().
 */
std::string safe_strerror(int errnum);

/**
 * Check if value is a power of two.
 */
bool isPowerOfTwo(int value);

/**
 * Round up value to the nearest power of two
 */
int roundUpToPowerOfTwo(int value);

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __UTIL_H__
