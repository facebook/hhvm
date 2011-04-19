/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <set>
#include <stdlib.h>
#include <stdarg.h>

/**
 * Simple utility functions.
 */
namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

#define ALWAYS_INLINE  __attribute__((always_inline))
#define LIKELY(pred)   __builtin_expect((pred), true)
#define UNLIKELY(pred) __builtin_expect((pred), false)

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
 * Drop the cached pages associated with the file from the file system cache.
 */
int drop_cache(int fd, off_t len = 0);
int drop_cache(FILE *f, off_t len = 0);

/**
 * Copy srcfile to dstfile, return 0 on success, -1 otherwise
 */
int copy(const char *srcfile, const char *dstfile);

/**
 * Like copy but using little disk-cache
 */
int directCopy(const char *srcfile, const char *dstfile);

/**
 * Like rename(2), but takes care of cross-filesystem rename.
 */
int rename(const char *oldname, const char *newname);

/**
 * Like rename but using little disk-cache
 */
int directRename(const char *oldname, const char *newname);

/**
 * Like system(3), but automatically print errors if execution fails.
 */
int ssystem(const char *command);

/**
 * Find the relative path from a directory with trailing slash to the file
 */
std::string relativePath(const std::string fromDir, const std::string toFile);

/**
 * Canonicalize path to remove "..", "." and "\/", etc..
 */
std::string canonicalize(const std::string &path);
const char *canonicalize(const char *path, size_t len);

/**
 * Makes sure there is ending slash by changing "path/name" to "path/name/".
 */
std::string normalizeDir(const std::string &dirname);

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
void string_printf(std::string &msg, const char *fmt, ...);
void string_vsnprintf(std::string &msg, const char *fmt, va_list ap);

/**
 * Escaping strings for code generation.
 */
std::string escapeStringForCPP(const char *input, int len,
                               bool* binary = NULL);
inline std::string escapeStringForCPP(const std::string &input,
                                      bool* binary = NULL) {
  return escapeStringForCPP(input.data(), input.length(), binary);
}
std::string escapeStringForPHP(const char *input, int len);
inline std::string escapeStringForPHP(const std::string &input) {
  return escapeStringForPHP(input.data(), input.length());
}

/**
 * Search for PHP or non-PHP files under a directory.
 */
void find(std::vector<std::string> &out,
          const std::string &root, const char *path, bool php,
          const std::set<std::string> *excludeDirs = NULL,
          const std::set<std::string> *excludeFiles = NULL);

/**
 * Format a regex pattern by surrounding with slashes and escaping pattern.
 */
std::string format_pattern(const std::string &pattern, bool prefixSlash);

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __UTIL_H__
