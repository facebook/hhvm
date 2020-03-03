/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_UTIL_MALLOC_SIZE_CLASS_H_
#define incl_HPHP_UTIL_MALLOC_SIZE_CLASS_H_

#include "hphp/util/alloc-defs.h" // must include before checking USE_JEMALLOC

#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

/*
 * Note: Some of the the functions in this file depend on the internal
 * implementation details of jemalloc, which are subject to change in new
 * jemalloc major versions.
 */

////////////////////////////////////////////////////////////////////////////////

/*
 * Call mallctl, reading/writing values of type <T> if out/in are non-null,
 * respectively.  Assert/log on error, depending on errOk.
 */
template <typename T, bool ErrOK>
int mallctlHelper(const char *cmd, T* out, T* in) {
#ifdef USE_JEMALLOC
  auto outLen = sizeof(T);
  int err = mallctl(cmd,
                    out, out ? &outLen : nullptr,
                    in, in ? sizeof(T) : 0);
  assert(err != 0 || outLen == sizeof(T));
#else
  int err = ENOENT;
#endif
  if (!ErrOK && err != 0) {
    char msg[128];
    std::snprintf(msg, sizeof(msg),
                  "mallctl(\"%s\") failed with error %d",
                  cmd, err);
    throw std::runtime_error{msg};
  }
  return err;
}

template <typename T, bool ErrOK = false>
int mallctlReadWrite(const char *cmd, T* out, T in) {
  return mallctlHelper<T, ErrOK>(cmd, out, &in);
}

template <typename T, bool ErrOK = false>
int mallctlRead(const char* cmd, T* out) {
  return mallctlHelper<T, ErrOK>(cmd, out, static_cast<T*>(nullptr));
}

template <typename T, bool ErrOK = false>
int mallctlWrite(const char* cmd, T in) {
  return mallctlHelper<T, ErrOK>(cmd, static_cast<T*>(nullptr), &in);
}

template <bool ErrOK = false> int mallctlCall(const char* cmd) {
  // Use <unsigned> rather than <void> to avoid sizeof(void).
  return mallctlHelper<unsigned, ErrOK>(cmd, nullptr, nullptr);
}

/*
 * jemalloc pprof utility functions.
 */
int jemalloc_pprof_enable();
int jemalloc_pprof_disable();
int jemalloc_pprof_dump(const std::string& prefix, bool force);

/**
 * Free all unused memory back to system. On error, returns false and, if
 * not null, sets an error message in *errStr.
 */
bool purge_all(std::string* errStr = nullptr);

#ifndef USE_JEMALLOC

template<size_t Size>
struct is_malloc_size_class : std::true_type {};

template<size_t Size>
struct next_malloc_size_class : std::integral_constant<size_t, Size> {};

#else

/*
 * Returns whether Size is a jemalloc size class.
 */
template<size_t Size>
struct is_malloc_size_class
  : std::integral_constant<bool,
      // Small classes:
         Size == 8
     || (Size <=  128 && !(Size % 16))
     || (Size <=  256 && !(Size % 32))
     || (Size <=  512 && !(Size % 64))
     || (Size <= 1024 && !(Size % 128))
     || (Size <= 2048 && !(Size % 256))
     || (Size <= 4096 && !(Size % 512))
     // Large:
     || (Size <= 4096 * 1024 && !(Size % 4096))
     // Huge:
     || !(Size % (4096 * 1024))
    >
{};

/*
 * Returns the next jemalloc size class larger than or equal to Size.
 */
template<size_t Size>
struct next_malloc_size_class {
private:
  template<size_t Multiple>
  struct round {
    static constexpr size_t value =
      !(Size % Multiple) ? Size : Size + Multiple - Size % Multiple;
  };

public:
  using type = typename std::integral_constant<size_t,
    // Small classes:
    Size <= 8    ? 8 :
    Size <= 16   ? 16 :
    Size <= 128  ? round< 16>::value :
    Size <= 256  ? round< 32>::value :
    Size <= 512  ? round< 64>::value :
    Size <= 1024 ? round<128>::value :
    Size <= 2048 ? round<256>::value :
    Size <= 4096 ? round<512>::value :
    // Large:
    Size <= 4096 * 1024 ? round<4096>::value :
    // Huge:
    round<4096 * 1024>::value
  >::type;
  static constexpr size_t value = type::value;

  static_assert(is_malloc_size_class<value>::value,
                "Bug in malloc-size-class.h");
};

void init_mallctl_mibs();

void mallctl_epoch();
size_t mallctl_pactive(unsigned arenaId);
size_t mallctl_pdirty(unsigned arenaId);
size_t mallctl_all_pdirty();

#endif

//////////////////////////////////////////////////////////////////////

}

#endif
