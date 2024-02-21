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

#include <cstdint>
#include <string>

#include <folly/json/dynamic.h>

namespace HPHP {

struct ArrayData;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct IsTypeStructProfile {

public:

  bool shouldOptimize() const;
  void update(const ArrayData* ad);
  static void reduce(IsTypeStructProfile& l, const IsTypeStructProfile& r);
  std::string toString() const;
  folly::dynamic toDynamic() const;

private:
  uint32_t m_class{0};
  uint32_t m_total{0};
};

///////////////////////////////////////////////////////////////////////////////

}}

