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

#include "hphp/runtime/vm/srckey.h"

#include <folly/Format.h>

#include "hphp/runtime/vm/hhbc.h"

namespace HPHP {

std::string SrcKey::showInst() const {
  if (prologue()) return "Prologue";
  auto const f = func();
  return instrToString(f->at(offset()), f);
}

std::string show(SrcKey sk) {
  auto func = sk.func();
  auto unit = sk.unit();
  const char *filepath = "*anonFile*";
  if (unit->origFilepath()->data() && unit->origFilepath()->size()) {
    filepath = unit->origFilepath()->data();
  }
  return folly::sformat("{}:{} in {}(id 0x{:#x})@{: >6}{}{}{}",
                        filepath, sk.lineNumber(),
                        func->fullName()->data(),
                        sk.funcID().toInt(), sk.printableOffset(),
                        resumeModeShortName(sk.resumeMode()),
                        sk.hasThis()  ? "t" : "",
                        sk.prologue() ? "p" : "");
}

std::string showShort(SrcKey sk) {
  if (!sk.valid()) return "<invalid SrcKey>";
  return folly::sformat(
    "{}(id {:#x})@{}{}{}{}",
    sk.func()->fullName(),
    sk.funcID(),
    sk.printableOffset(),
    resumeModeShortName(sk.resumeMode()),
    sk.hasThis()  ? "t" : "",
    sk.prologue() ? "p" : ""
  );
}

void sktrace(SrcKey sk, const char *fmt, ...) {
  if (!Trace::enabled) return;

  auto const inst = sk.showInst();
  Trace::trace("%s: %20s ", show(sk).c_str(), inst.c_str());
  va_list a;
  va_start(a, fmt);
  Trace::vtrace(fmt, a);
  va_end(a);
}

std::string SrcKey::getSymbol() const {
  const Func* f = func();

  if (f->isBuiltin()) {
    return f->fullName()->data();
  }

  if (f->isMethod() && !f->cls()) {
    return folly::format(
      "{}::{}::line-{}",
      f->preClass()->name(),
      f->name(),
      lineNumber()
    ).str();
  }

  // methods with a cls() and functions
  return folly::format(
    "{}::line-{}",
    f->fullName(),
    lineNumber()
  ).str();
}

}
