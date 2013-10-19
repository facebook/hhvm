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

#include "hphp/runtime/vm/srckey.h"

#include "folly/Format.h"

#include "hphp/runtime/vm/hhbc.h"

namespace HPHP {

std::string SrcKey::showInst() const {
  return instrToString(reinterpret_cast<const Op*>(unit()->at(offset())));
}

std::string show(SrcKey sk) {
  auto func = sk.func();
  auto unit = sk.unit();
  const char *filepath = "*anonFile*";
  if (unit->filepath()->data() && unit->filepath()->size()) {
    filepath = unit->filepath()->data();
  }
  return folly::format("{}:{} in {}(id 0x{:#x})@{: >6}",
                       filepath, unit->getLineNumber(sk.offset()),
                       func->isPseudoMain() ? "pseudoMain"
                                            : func->fullName()->data(),
                       (unsigned long long)sk.getFuncId(), sk.offset()).str();
}

std::string showShort(SrcKey sk) {
  if (!sk.valid()) return "<invalid SrcKey>";
  return folly::format("{}(id {:#x})@{}",
                       sk.func()->fullName()->data(), sk.getFuncId(),
                       sk.offset()).str();
}

void sktrace(SrcKey sk, const char *fmt, ...) {
  if (!Trace::enabled) return;

  auto inst = instrToString((Op*)sk.unit()->at(sk.offset()));
  Trace::trace("%s: %20s ", show(sk).c_str(), inst.c_str());
  va_list a;
  va_start(a, fmt);
  Trace::vtrace(fmt, a);
  va_end(a);
}

std::string SrcKey::getSymbol() const {
  const Func* f = func();
  const Unit* u = unit();

  if (f->isBuiltin()) {
    return f->fullName()->data();
  }

  if (f->isPseudoMain()) {
    return folly::format(
      "{{pseudo-main}}::{}::line-{}",
      u->filepath()->data(),
      u->getLineNumber(m_offset)
    ).str();
  }

  if (f->isMethod() && !f->cls()) {
    return folly::format(
      "{}::{}::line-{}",
      f->preClass()->name()->data(),
      f->name()->data(),
      u->getLineNumber(m_offset)
    ).str();
  }

  // methods with a cls() and functions
  return folly::format(
    "{}::line-{}",
    f->fullName()->data(),
    u->getLineNumber(m_offset)
  ).str();
}

}
