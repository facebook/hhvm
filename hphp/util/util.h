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

/**
 * Simple utility functions.
 */
namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Non-gcc compat
#ifndef __GNUC__
# define __attribute__(x)
#endif

#define ATTRIBUTE_UNUSED   __attribute__((__unused__))
#define ATTRIBUTE_NORETURN __attribute__((__noreturn__))
#ifndef ATTRIBUTE_PRINTF
# if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ > 6
#  define ATTRIBUTE_PRINTF(a1,a2) \
     __attribute__((__format__ (__printf__, a1, a2)))
# else
#  define ATTRIBUTE_PRINTF(a1,a2)
# endif
#endif

#define ALWAYS_INLINE      inline __attribute__((__always_inline__))
#define NEVER_INLINE       __attribute__((__noinline__))
#define INLINE_SINGLE_CALLER ALWAYS_INLINE
#define UNUSED             __attribute__((__unused__))
#define FLATTEN            __attribute__((__flatten__))
#define EXTERNALLY_VISIBLE __attribute__((__externally_visible__))
#if FACEBOOK
#define AT_END_OF_TEXT       __attribute__((__section__(".stub")))
#else
#define AT_END_OF_TEXT
#endif

#ifdef DEBUG
# define DEBUG_ONLY /* nop */
#else
# define DEBUG_ONLY UNUSED
#endif

/*
 * We need to keep some unreferenced functions from being removed by
 * the linker. There is no compile time mechanism for doing this, but
 * by putting them in the same section as some other, referenced function
 * in the same file, we can keep them around.
 *
 * So this macro should be used to mark at least one function that is
 * referenced, and other functions that are not referenced in the same
 * file.
 *
 * Note: this may not work properly with LTO. We'll revisit when/if we
 * move to it.
 */
#ifndef __APPLE__
# define KEEP_SECTION \
    __attribute__((__section__(".text.keep")))
#else
# define KEEP_SECTION \
    __attribute__((__section__(".text.keep,")))
#endif

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
 * Check if value is a power of two.
 */
template<typename Int>
inline bool isPowerOfTwo(Int value) {
  return (value > 0 && (value & (value-1)) == 0);
}

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

// Returns the smallest power of 2 >= value, so long as value is not zero
inline uint64_t nextPower2(uint64_t value) {
  return uint64_t(1) << lgNextPower2(value);
}

inline uint32_t nextPower2(uint32_t value) {
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
void string_printf(std::string &msg,
                   const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
void string_vsnprintf(std::string &msg,
                      const char *fmt, va_list ap) ATTRIBUTE_PRINTF(2,0);

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

/**
 * Portable macros for mapping machine stack and frame pointers to variables.
 */
#if defined(__x86_64__)

#  define DECLARE_STACK_POINTER(sp)               \
     void* sp;                                    \
     asm volatile("mov %%rsp, %0" : "=r" (sp) ::)

#  if defined(__clang__)
#    define DECLARE_FRAME_POINTER(fp)               \
       ActRec* fp;                                  \
       asm volatile("mov %%rbp, %0" : "=r" (fp) ::)
#  else
#    define DECLARE_FRAME_POINTER(fp) register ActRec* fp asm("rbp");
#  endif

#elif defined(__AARCH64EL__)

#  if defined(__clang__)
#    error Clang implementation not done for ARM
#  endif

#  define DECLARE_STACK_POINTER(sp) register void*   sp asm("sp");
#  define DECLARE_FRAME_POINTER(fp) register ActRec* fp asm("x29");

#else

#  error What are the stack and frame pointers called on your architecture?

#endif

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

class LogFileFlusher {
public:
  LogFileFlusher() : m_bytesWritten(0) {}
  virtual ~LogFileFlusher() {}

  void recordWriteAndMaybeDropCaches(int fd, int bytes);
  inline void recordWriteAndMaybeDropCaches(FILE* f, int bytes) {
    recordWriteAndMaybeDropCaches(fileno(f), bytes);
  }

  enum {
    kDropCacheTail = 1 * 1024 * 1024
  };

  static int DropCacheChunkSize;
protected:
  // For testing
  virtual void dropCache(int fd, off_t len) {
    Util::drop_cache(fd, len);
  }

private:
  std::atomic<int> m_bytesWritten;
};
}

#endif
