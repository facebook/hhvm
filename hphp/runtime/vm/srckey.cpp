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

namespace HPHP {

void sktrace(SrcKey sk, const char *fmt, ...) {
  if (!Trace::enabled) {
    return;
  }
  // We don't want to print string literals, so don't pass the unit
  auto func = sk.func();
  auto unit = sk.unit();
  string s = instrToString((Op*)unit->at(sk.offset()));
  const char *filepath = "*anonFile*";
  if (unit->filepath()->data() && strlen(unit->filepath()->data()) > 0) {
    filepath = unit->filepath()->data();
  }
  Trace::trace("%s:%d in %s(id 0x%llx) %6d: %20s ",
               filepath, unit->getLineNumber(sk.offset()),
               func->isPseudoMain() ? "pseudoMain" : func->fullName()->data(),
               (unsigned long long)sk.getFuncId(), sk.offset(), s.c_str());
  va_list a;
  va_start(a, fmt);
  Trace::vtrace(fmt, a);
  va_end(a);
}

}
