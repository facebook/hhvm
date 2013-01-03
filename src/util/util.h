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

#ifndef incl_HPHP_UTIL_H_
#define incl_HPHP_UTIL_H_

#include <vector>
#include <string>
#include <map>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h> // For htonl().

/**
 * Simple utility functions.
 */
namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

#define ALWAYS_INLINE  __attribute__((always_inline))
#define NEVER_INLINE  __attribute__((noinline))
#define INLINE_SINGLE_CALLER ALWAYS_INLINE
#define LIKELY(pred)   __builtin_expect((pred), true)
#define UNLIKELY(pred) __builtin_expect((pred), false)
#define UNUSED         __attribute__((unused))
#define FLATTEN        __attribute__((flatten))
#define HOT_FUNC       __attribute__ ((section (".text.hot.builtin")))

#ifdef DEBUG
#define DEBUG_ONLY /* nop */
#else
#define DEBUG_ONLY UNUSED
#endif

#ifdef HHVM
#define HOT_FUNC_VM HOT_FUNC
#define HOT_FUNC_HPHP
#else
#define HOT_FUNC_VM
#define HOT_FUNC_HPHP HOT_FUNC
#endif

/*
 * we need to keep some unreferenced functions from being removed by
 * the linker. There is no compile time mechanism for doing this, but
 * by putting them in the same section as some other, referenced function
 * in the same file, we can keep them around.
 *
 * So this macro should be used to mark at least one function that is
 * referenced, and other functions that are not referenced in the same
 * file
 */
#define KEEP_SECTION \
  __attribute__((externally_visible))

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
 * Check if value is a power of two.
 */
template<typename Int>
static inline bool isPowerOfTwo(Int value) {
  return (value > 0 && (value & (value-1)) == 0);
}

/**
 * Round up value to the nearest power of two
 */
template<typename Int>
static inline Int roundUpToPowerOfTwo(Int value) {
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
static inline int lgNextPower2(uint64_t value) {
#ifdef DEBUG
  (void) (0 / value); // fail for 0; ASSERT is a pain.
#endif
  return 64 - __builtin_clzll(value - 1);
}

static inline int lgNextPower2(uint32_t value) {
#ifdef DEBUG
  (void) (0 / value); // fail for 0; ASSERT is a pain.
#endif
  return 32 - __builtin_clz(value - 1);
}

static inline uint64_t nextPower2(uint64_t value) {
  return uint64_t(1) << lgNextPower2(value);
}

static inline uint32_t nextPower2(uint32_t value) {
  return uint32_t(1) << lgNextPower2(value);
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

static inline void compiler_membar( ) {
  asm volatile("" : : :"memory");
}

/**
 * Given the address of a C++ function, returns that function's name
 * or a hex string representation of the address if it can't find
 * the function's name. Attempts to demangle C++ function names. It's
 * the caller's responsibility to free the returned C string.
 */
char* getNativeFunctionName(void* codeAddr);

/**
 * Get the vtable offset corresponding to a method pointer. NB: only works
 * for single inheritance. For no inheritance at all, use
 * getMethodHardwarePtr. ABI-specific, don't play on or around.
 */
template <typename MethodPtr>
int64_t getVTableOffset(MethodPtr meth) {
  union {
    MethodPtr meth;
    int64_t offset;
  } u;
  u.meth = meth;
  return u.offset - 1;
}

template <typename MethodPtr>
void* getMethodHardwarePtr(MethodPtr meth) {
  union {
    MethodPtr meth;
    void* ptr;
  } u;
  u.meth = meth;
  return u.ptr;
}

/**
 * 64-bit equivalents of 32-bit htonl() and ntohq().
 */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htonq(a) a
#define ntohq(a) a
#else
#define ntohq(a)                                                              \
  (uint64_t)(((uint64_t) (ntohl((uint32_t) ((a) >> 32))))                     \
             | (((uint64_t) (ntohl((uint32_t)                                 \
                ((a) & 0x00000000ffffffff)))) << 32))
#define htonq(a)                                                              \
  (uint64_t) (((uint64_t) (htonl((uint32_t) ((a) >> 32))))                    \
              | (((uint64_t) (htonl((uint32_t)                                \
                 ((a) & 0x00000000ffffffff)))) << 32))
#endif

/**
 * Read typed data from an offset relative to a base address
 */
template <class T>
static T& getDataRef(void* base, unsigned offset) {
  return *(T*)((char*)base + offset);
}

///////////////////////////////////////////////////////////////////////////////
}}

#endif
