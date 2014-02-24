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

//////////////////////////////////////////////////////////////////////

/*
 * This header is deprecated, please don't include it in anything new
 * or add new code to this header.
 *
 * If you need something defined in this header, pull it out to a
 * smaller header and include that.
 *
 * TODO(#3468751): split this header up
 */

//////////////////////////////////////////////////////////////////////

#ifndef incl_HPHP_UTIL_H_
#define incl_HPHP_UTIL_H_

#include <cassert>
#include <atomic>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h> // For htonl().

#include "folly/Likely.h"

#include "hphp/util/portability.h"
#include "hphp/util/string-vsnprintf.h"

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
std::string canonicalize(const std::string& path);
char* canonicalize(const char* path, size_t len,
                   bool collapse_slashes = true);

/**
 * Makes sure there is ending slash by changing "path/name" to "path/name/".
 */
std::string normalizeDir(const std::string &dirname);

/**
 * Thread-safe dirname().
 */
std::string safe_dirname(const char *path, int len);
std::string safe_dirname(const char *path);
std::string safe_dirname(const std::string& path);

/**
 * Helper function for safe_dirname.
 */
size_t dirname_helper(char *path, int len);

/**
 * Round up value to the nearest power of two
 */
template<typename Int>
inline Int roundUpToPowerOfTwo(Int value) {
#ifdef DEBUG
  (void) (0 / value); // fail for 0; ASSERT is a pain.
#endif
  --value;
  // Verified that gcc is smart enough to unroll this and emit
  // constant shifts.
  for (unsigned i = 1; i < sizeof(Int) * 8; i *= 2)
    value |= value >> i;
  ++value;
  return value;
}

/**
 * Return log-base-2 of the next power of 2, i.e. CLZ
 */
inline int lgNextPower2(uint64_t value) {
  assert(value != 0);
#ifdef __x86_64__
  // __builtin_clz emits the bsr instruction, but doesn't let us pass -1
  // through for the 0 case.  Also, it is careful to convert bsr to clz
  // with 6-bit arithmetic, which is not necessary here
  uint64_t result = -1;
  --value;
  asm ("bsr %2, %0" : "=r" (result) : "0" (result), "r" (value) : "flags");
  return result + 1;
#else
  return value == 1 ? 0 : 64 - __builtin_clzll(value - 1);
#endif
}

inline int lgNextPower2(uint32_t value) {
  assert(value != 0);
#ifdef __x86_64__
  uint32_t result = -1;
  --value;
  asm ("bsr %2, %0" : "=r" (result) : "0" (result), "r" (value) : "flags");
  return result + 1;
#else
  return value == 1 ? 0 : 32 - __builtin_clz(value - 1);
#endif
}

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
 * Search for PHP or non-PHP files under a directory.
 */
void find(std::vector<std::string> &out,
          const std::string &root, const char *path, bool php,
          const std::set<std::string> *excludeDirs = nullptr,
          const std::set<std::string> *excludeFiles = nullptr);

/**
 * Format a regex pattern by surrounding with slashes and escaping pattern.
 */
std::string format_pattern(const std::string &pattern, bool prefixSlash);

inline void assert_native_stack_aligned() {
#ifndef NDEBUG
  DECLARE_STACK_POINTER(sp);
  assert(reinterpret_cast<uintptr_t>(sp) % 16 == 0);
#endif
}

/**
 * Read typed data from an offset relative to a base address
 */
template <class T>
T& getDataRef(void* base, unsigned offset) {
  return *(T*)((char*)base + offset);
}

///////////////////////////////////////////////////////////////////////////////
}
}

#endif
