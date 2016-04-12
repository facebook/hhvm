/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/type-source.h"

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

#include <folly/Format.h>

#include <string>

namespace HPHP { namespace jit {

bool TypeSource::operator<(const TypeSource& rhs) const {
  if (kind != rhs.kind) {
    return int(kind) < int(rhs.kind);
  }
  if (isGuard()) return guard->id() < rhs.guard->id();
  assertx(isValue());
  return value->id() < rhs.value->id();
}

std::string TypeSource::toString() const {
  return show(*this);
}

std::string show(const TypeSource& typeSrc) {
  if (typeSrc.isGuard()) return typeSrc.guard->toString();
  if (typeSrc.isValue()) return typeSrc.value->toString();
  always_assert(false);
}

std::string show(const TypeSourceSet& typeSrcs) {
  std::string ret;
  for (auto& typeSrc : typeSrcs) {
    ret += show(typeSrc) + ", ";
  }
  return ret;
}

}}
