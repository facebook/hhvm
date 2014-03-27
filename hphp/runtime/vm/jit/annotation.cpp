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

#include "folly/MapUtil.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/repo.h"

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(trans);

const StaticString s_empty("");

/*
 * A mapping from FCall instructions to the statically-known Func* that they're
 * calling. Used to accelerate our FCall translations.
 */
typedef hphp_hash_map<SrcKey,const Func*,SrcKey::Hasher> CallDB;
static CallDB s_callDB;

static void recordFunc(const SrcKey sk,
                       const Func* func) {
  FTRACE(2, "annotation: recordFunc: {}@{} {}\n",
         sk.unit()->filepath()->data(),
         sk.offset(),
         func->fullName()->data());

  s_callDB.insert(std::make_pair(sk, func));
}

static const Func* lookupDirectFunc(SrcKey const sk,
                                    const StringData* fname,
                                    const StringData* clsName,
                                    bool staticCall) {
  if (clsName && !clsName->isame(s_empty.get())) {
    auto const cls = Unit::lookupUniqueClass(clsName);
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

static void recordActRecPush(const SrcKey sk,
                             const StringData* name,
                             const StringData* clsName,
                             bool staticCall) {
  auto unit = sk.unit();
  FTRACE(2, "annotation: recordActRecPush: {}@{} {}{}{} ({}static)\n",
         unit->filepath()->data(),
         sk.offset(),
         clsName ? clsName->data() : "",
         clsName ? "::" : "",
         name,
         !staticCall ? "non" : "");

  SrcKey next(sk);
  next.advance(unit);
  const FPIEnt *fpi = sk.func()->findFPI(next.offset());
  assert(fpi);
  assert(name->isStatic());
  assert(sk.offset() == fpi->m_fpushOff);
  auto const fcall = SrcKey { sk.func(), fpi->m_fcallOff };
  assert(isFCallStar(*reinterpret_cast<const Op*>(unit->at(fcall.offset()))));
  auto const func = lookupDirectFunc(sk, name, clsName, staticCall);
  if (func) {
    recordFunc(fcall, func);
  }
}

void annotate(NormalizedInstruction* i) {
  switch(i->op()) {
    case OpFPushObjMethodD:
    case OpFPushClsMethodD:
    case OpFPushClsMethodF:
    case OpFPushCtorD:
    case OpFPushCtor:
    case OpFPushFuncD: {
      if (RuntimeOption::RepoAuthoritative && Repo::global().UsedHHBBC) {
        break;
      }

      // When we push predictable activation records, we can use a simpler
      // translation for their corresponding FCall.
      const StringData* className = nullptr;
      const StringData* funcName = nullptr;
      if (i->op() == OpFPushFuncD) {
        funcName = i->m_unit->lookupLitstrId(i->imm[1].u_SA);
      } else if (i->op() == OpFPushObjMethodD) {
        if (i->inputs[0]->valueType() != KindOfObject) break;
        const Class* cls = i->inputs[0]->rtt.valueClass();
        if (!cls) break;
        funcName = i->m_unit->lookupLitstrId(i->imm[1].u_SA);
        className = cls->name();
      } else if (i->op() == OpFPushClsMethodF) {
        if (!i->inputs[1]->isString() ||
            i->inputs[1]->rtt.valueString() == nullptr ||
            i->inputs[0]->valueType() != KindOfClass) {
          break;
        }
        const Class* cls = i->inputs[0]->rtt.valueClass();
        if (!cls) break;
        funcName = i->inputs[1]->rtt.valueString();
        className = cls->name();
      } else if (i->op() == OpFPushClsMethodD) {
        funcName = i->m_unit->lookupLitstrId(i->imm[1].u_SA);
        className = i->m_unit->lookupLitstrId(i->imm[2].u_SA);
      } else if (i->op() == OpFPushCtorD) {
        className = i->m_unit->lookupLitstrId(i->imm[1].u_SA);
        const Class* cls = Unit::lookupUniqueClass(className);
        if (!cls) break;
        funcName = cls->getCtor()->name();
      } else {
        assert(i->op() == OpFPushCtor);
        const Class* cls = i->inputs[0]->rtt.valueClass();
        if (!cls) break;
        funcName = cls->getCtor()->name();
      }
      assert(funcName->isStatic());
      recordActRecPush(i->source, funcName, className,
                       i->op() == OpFPushClsMethodD ||
                       i->op() == OpFPushClsMethodF);
    } break;
    case OpFCall:
    case OpFCallArray: {
      if (RuntimeOption::RepoAuthoritative && Repo::global().UsedHHBBC) {
        break;
      }
      if (auto const func = folly::get_ptr(s_callDB, i->source)) {
        i->funcd = *func;
      }
    } break;
    default: break;

    case Op::FCallD: {
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
          : lookupDirectFunc(i->source, funcName, clsName, isStatic);

      if (func) {
        FTRACE(1, "found direct func (%s) for FCallD\n",
          func->fullName()->data());
        i->funcd = func;
      }
      break;
    }
  }
}

const StringData*
fcallToFuncName(const NormalizedInstruction* i) {
  if (i->op() == Op::FCallD) {
    return i->m_unit->lookupLitstrId(i->imm[2].u_SA);
  }
  if (RuntimeOption::RepoAuthoritative && Repo::global().UsedHHBBC) {
    return nullptr;
  }
  if (auto const func = folly::get_ptr(s_callDB, i->source)) {
    return (*func)->name();
  }
  return nullptr;
}

} }

