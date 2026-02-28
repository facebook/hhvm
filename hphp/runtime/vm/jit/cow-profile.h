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

#pragma once

#include <folly/json/dynamic.h>

#include <cstdint>

namespace HPHP {

struct ArrayData;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

// Profile whether an array could need to COW before a mutation
struct COWProfile {

  enum class Result {
    AlwaysCOW = 0,
    UsuallyCOW,
    None,
    RarelyCOW,
    NeverCOW
  };

  Result choose() const;

  void update(const ArrayData* ad);

  static void reduce(COWProfile& l, const COWProfile& r);

  std::string toString() const;
  folly::dynamic toDynamic() const;

private:
  uint64_t m_nocow{0};
  uint64_t m_total{0};
};

///////////////////////////////////////////////////////////////////////////////

}}
