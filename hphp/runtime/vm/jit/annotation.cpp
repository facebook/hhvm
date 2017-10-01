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

#include "hphp/runtime/vm/jit/annotation.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(trans);

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_empty("");
const Func* lookupDirectFunc(SrcKey const sk,
                             const StringData* fname,
                             const StringData* clsName,
                             Op pushOp) {
  if (clsName && !clsName->empty()) {
    auto const cls = Unit::lookupUniqueClassInContext(clsName,
                                                      sk.func()->cls());
    bool magic = false;
    auto const isExact =
      pushOp == Op::FPushClsMethodD ||
      pushOp == Op::FPushClsMethodF;
    auto const isStatic = isExact || pushOp == Op::FPushClsMethod;
    auto const func = lookupImmutableMethod(cls, fname, magic,
                                            isStatic, sk.func(), isExact);
    if (func &&
        !isExact &&
        !func->isImmutableFrom(cls) &&
        (isStatic || !(func->attrs() & AttrPrivate))) return nullptr;
    return func;
  }
  return lookupImmutableFunc(sk.unit(), fname).func;
}

const Func* lookupDirectCtor(SrcKey const sk,
                             const StringData* clsName,
                             Op pushOp) {
  if (clsName && !clsName->isame(s_empty.get())) {
    auto const ctx = sk.func()->cls();
    auto const cls = Unit::lookupUniqueClassInContext(clsName, ctx);
    auto const func = lookupImmutableCtor(cls, ctx);
    if (!func ||
        pushOp == Op::FPushCtorD ||
        cls->attrs() & AttrNoOverride) {
      return func;
    }
  }

  return nullptr;
}

const void annotate(NormalizedInstruction* i,
                    const StringData* clsName, const StringData* funcName) {
  auto const fpi      = i->func()->findFPI(i->source.offset());
  auto pc             = i->m_unit->at(fpi->m_fpushOff);
  auto const pushOp   = decode_op(pc);

  auto decode_litstr = [&] {
    Id id;
    std::memcpy(&id, pc, sizeof id);
    pc += sizeof id;
    return i->m_unit->lookupLitstrId(id);
  };

  if (!funcName && !clsName) {
    switch (pushOp) {
      case Op::FPushClsMethodD:
        decode_iva(pc);
        funcName = decode_litstr();
        clsName = decode_litstr();
        break;
      case Op::FPushFuncD:
        decode_iva(pc);
        funcName = decode_litstr();
        clsName = nullptr;
        break;
      case Op::FPushCtorD:
        decode_iva(pc);
        clsName = decode_litstr();
        break;
      default:
        return;
    }
  }

  auto const func =
    pushOp == Op::FPushCtorD || pushOp == Op::FPushCtor ?
    lookupDirectCtor(i->source, clsName, pushOp) :
    lookupDirectFunc(i->source, funcName, clsName, pushOp);

  if (func) {
    FTRACE(1, "found direct func ({}) for FCallD\n",
           func->fullName()->data());
    i->funcd = func;
  }
}

}

//////////////////////////////////////////////////////////////////////

void annotate(NormalizedInstruction* i) {
  switch (i->op()) {
  case Op::FCall:
    annotate(i, nullptr, nullptr);
    break;
  case Op::FCallD:
    annotate(i,
             i->m_unit->lookupLitstrId(i->imm[1].u_SA),
             i->m_unit->lookupLitstrId(i->imm[2].u_SA));
    break;
  default:
    break;
  }
}

//////////////////////////////////////////////////////////////////////

}}
