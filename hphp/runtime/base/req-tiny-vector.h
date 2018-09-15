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
#ifndef incl_HPHP_RUNTIME_BASE_REQ_TINY_VECTOR_H_
#define incl_HPHP_RUNTIME_BASE_REQ_TINY_VECTOR_H_

#include "hphp/runtime/base/req-malloc.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/type-scan.h"

namespace HPHP { namespace req {

// Special allocator for TinyVector using request heap allocation.
template <typename T, typename Element = T> struct TinyVectorReqAllocator {
  using value_type = T;
  using pointer = T*;
  using size_type = std::size_t;

  template <typename U> struct rebind {
    using other = TinyVectorReqAllocator<U, Element>;
  };

  pointer allocate(size_type size) {
    return reinterpret_cast<pointer>(
        req::malloc(size,
                    type_scan::getIndexForMalloc<
                    T, type_scan::Action::Conservative<Element>>())
    );
  }
  void deallocate(pointer ptr, size_type /* size */) {
    req::free(ptr);
  }
};

template<typename T,
         std::size_t Internal = 1,
         std::size_t MinHeap = 0>
struct TinyVector final : HPHP::TinyVector<T,
                                           Internal,
                                           MinHeap,
                                           TinyVectorReqAllocator<T>> {
  using Base = HPHP::TinyVector<T,Internal,MinHeap,TinyVectorReqAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

}}

#endif
