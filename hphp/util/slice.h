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
#ifndef incl_HPHP_UTIL_SLICE_H_
#define incl_HPHP_UTIL_SLICE_H_

#include <cinttypes>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * A Slice is a compact way to refer to an extent of array elements.
 * For hhvm, it's generally string slices.
 *
 * This type is designed to be passed around by value.  Methods on
 * slice are set up to match the Boost RandomAccessRange concept.  If
 * T is not const-qualified, it also models WriteableRange.
 */
template<class T>
struct Slice {
  typedef T* iterator;
  typedef T* const_iterator;
  typedef uint32_t size_type;

  Slice(T* ptr, uint32_t len) : ptr(ptr), len(len) {}

  const_iterator begin() const { return ptr; }
  const_iterator end() const { return ptr + len; }

  size_type size() const { return len; }

  T* ptr;         // pointer to bytes, not necessarily \0 teriminated
  size_type len;  // number of bytes, not counting possible \0
};

typedef Slice<const char> StringSlice;
typedef Slice<char> MutableSlice;

//////////////////////////////////////////////////////////////////////

}

#endif
