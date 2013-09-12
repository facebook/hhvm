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
#ifndef incl_HPHP_UTIL_ABI_CXX_H_
#define incl_HPHP_UTIL_ABI_CXX_H_

#include <inttypes.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Given the address of a C++ function, returns that function's name
 * or a hex string representation of the address if it can't find
 * the function's name. Attempts to demangle C++ function names. It's
 * the caller's responsibility to free the returned C string.
 */
char* getNativeFunctionName(void* codeAddr);

/**
 * Get the vtable offset corresponding to a method pointer. NB: only works
 * for single inheritance. For no inheritance at all, use
 * getMethodPtr. ABI-specific, don't play on or around.
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
union MethodPtrU {
  MethodPtr meth;
  void* ptr;
};

template <typename MethodPtr>
constexpr void* getMethodPtr(MethodPtr meth) {
  return ((MethodPtrU<MethodPtr>*)(&meth))->ptr;
}

//////////////////////////////////////////////////////////////////////

}


#endif
