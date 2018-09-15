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
#ifndef incl_HPHP_RUNTIME_BASE_REQ_VECTOR_H_
#define incl_HPHP_RUNTIME_BASE_REQ_VECTOR_H_

#include "hphp/runtime/base/req-malloc.h"
#include "hphp/util/type-scan.h"
#include <vector>

namespace HPHP { namespace req {

template <typename T>
struct vector final : std::vector<T, ConservativeAllocator<T>> {
  using Base = std::vector<T, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    // std::vector guarantees the values are contiguous
    if (auto n = Base::size()) {
      scanner.scan(*Base::data(), n * sizeof(T));
    }
  }
};

}}

#endif
