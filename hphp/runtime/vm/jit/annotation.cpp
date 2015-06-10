/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/annotation.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(trans);

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_empty("");
const Func* lookupDirectFunc(SrcKey const sk,
                             const StringData* fname,
                             const StringData* clsName,
                             bool staticCall) {
  if (clsName && !clsName->empty()) {
    auto const cls = Unit::lookupClassOrUniqueClass(clsName);
    bool magic = false;
    auto const ctx = sk.func()->cls();
    return lookupImmutableMethod(cls, fname, magic, staticCall, ctx);
  }
  auto const func = Unit::lookupFunc(fname);
  if (func && func->isNameBindingImmutable(sk.unit())) {
    return func;
  }
  return nullptr;
}

const Func* lookupDirectCtor(SrcKey const sk, const StringData* clsName) {
  if (clsName && !clsName->isame(s_empty.get())) {
    auto const cls = Unit::lookupClassOrUniqueClass(clsName);
    auto const ctx = sk.func()->cls();
    return lookupImmutableCtor(cls, ctx);
  }

  return nullptr;
}

}

//////////////////////////////////////////////////////////////////////

void annotate(NormalizedInstruction* i) {
  switch (i->op()) {
  case Op::FCallD:
    {
      auto const fpi      = i->func()->findFPI(i->source.offset());
      auto const pushOp   = i->m_unit->getOpcode(fpi->m_fpushOff);
      auto const clsName  = i->m_unit->lookupLitstrId(i->imm[1].u_SA);
      auto const funcName = i->m_unit->lookupLitstrId(i->imm[2].u_SA);
      auto const isStatic = pushOp == Op::FPushClsMethodD ||
                            pushOp == Op::FPushClsMethodF ||
                            pushOp == Op::FPushClsMethod;

      /*
       * Currently we don't attempt any of this for FPushClsMethod
       * because lookupImmutableMethod is only for situations that
       * don't involve LSB.
       */
      auto const func =
        pushOp == Op::FPushClsMethod
          ? nullptr
          : pushOp == Op::FPushCtorD
            ? lookupDirectCtor(i->source, clsName)
            : lookupDirectFunc(i->source, funcName, clsName, isStatic);

      if (func) {
        FTRACE(1, "found direct func ({}) for FCallD\n",
          func->fullName()->data());
        i->funcd = func;
      }
    }
    break;
  default:
    break;
  }
}

//////////////////////////////////////////////////////////////////////

}}

