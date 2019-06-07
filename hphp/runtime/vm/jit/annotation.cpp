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
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(trans);

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_empty("");
const Func* lookupDirectFunc(SrcKey const sk,
                             const StringData* fname,
                             const StringData* clsName,
                             bool isExact,
                             bool isStatic) {
  if (clsName && !clsName->empty()) {
    auto const cls = Unit::lookupUniqueClassInContext(clsName,
                                                      sk.func()->cls());
    if (!cls || isInterface(cls)) return nullptr;
    if (isStatic) {
      auto const f = lookupImmutableClsMethod(cls, fname, sk.func(), isExact);
      if (f && !isExact && !f->isImmutableFrom(cls)) return nullptr;
      return f;
    } else {
      auto const l = lookupImmutableObjMethod(cls, fname, sk.func(), isExact);
      if (l.type == ImmutableObjMethodLookup::Type::Func ||
          l.type == ImmutableObjMethodLookup::Type::MagicFunc) {
        return l.func;
      }
      return nullptr;
    }
  }
  return lookupImmutableFunc(sk.unit(), fname).func;
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

  if (funcName->empty() && clsName->empty()) {
    switch (pushOp) {
      case Op::FPushClsMethodD:
      case Op::FPushClsMethodRD:
        decode_iva(pc);
        funcName = decode_litstr();
        clsName = decode_litstr();
        break;
      case Op::FPushFuncD:
      case Op::FPushFuncRD:
        decode_iva(pc);
        funcName = decode_litstr();
        clsName = nullptr;
        break;
      default:
        return;
    }
  }

  bool isStatic = false;
  bool isExact = false;
  switch (pushOp) {
    case Op::FPushClsMethodD:
    case Op::FPushClsMethodRD:
      isExact = true;
      isStatic = true;
      break;
    case Op::FPushClsMethod:
      isStatic = true;
      break;
    case Op::FPushClsMethodS:
    case Op::FPushClsMethodSD: {
      decode_iva(pc);
      auto const ref = decode_oa<SpecialClsRef>(pc);
      isExact = (ref == SpecialClsRef::Self) || (ref == SpecialClsRef::Parent);
      isStatic = true;
      break;
    }
    default:
      break;
  }

  auto const func =
    lookupDirectFunc(i->source, funcName, clsName, isExact, isStatic);

  if (func) {
    FTRACE(1, "found direct func ({}) for FCall\n",
           func->fullName()->data());
    i->funcd = func;
  }
}

}

//////////////////////////////////////////////////////////////////////

void annotate(NormalizedInstruction* i) {
  if (!isLegacyFCall(i->op())) return;
  annotate(i,
           i->m_unit->lookupLitstrId(i->imm[1].u_SA),
           i->m_unit->lookupLitstrId(i->imm[2].u_SA));
}

//////////////////////////////////////////////////////////////////////

}}
