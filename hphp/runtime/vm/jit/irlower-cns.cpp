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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/cls-cns-profile.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdCns(IRLS& env, const IRInstruction* inst) {
  auto const cnsName = inst->src(0)->strVal();
  auto const ch = makeCnsHandle(cnsName);
  assertx(rds::isHandleBound(ch));
  auto const dst = dstLoc(env, inst, 0);
  auto& v = vmain(env);
  assertx(inst->taken());

  auto checkUninit = [&] {
    auto const sf = v.makeReg();
    irlower::emitTypeTest(
      v, env, TUninit, dst.reg(1), dst.reg(0), sf,
      [&] (ConditionCode cc, Vreg sfr) {
        fwdJcc(v, env, cc, sfr, inst->taken());
      }
    );
  };

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    fwdJcc(v, env, CC_NE, sf, inst->taken());
    loadTV(v, inst->dst(), dst, rvmtl()[ch]);

    // When a CLIServer is active requests running in script mode will define
    // the stdio constants which require lookup via special callbacks. To not
    // interfere with the server these constants will be defined as
    // non-persistent.
    if (!RuntimeOption::RepoAuthoritative) {
      if (strcasecmp(cnsName->data(), "stdin") == 0 ||
          strcasecmp(cnsName->data(), "stdout") == 0 ||
          strcasecmp(cnsName->data(), "stderr") == 0) {
        checkUninit();
      }
    }
    return;
  }

  auto const pcns = rds::handleToPtr<TypedValue, rds::Mode::Persistent>(ch);

  if (pcns->m_type == KindOfUninit) {
    loadTV(v, inst->dst(), dst, *v.cns(pcns));
    checkUninit();
  } else {
    // Statically known constant.
    assertx(!dst.isFullSIMD());
    switch (pcns->m_type) {
      case KindOfNull:
        v << copy{v.cns(nullptr), dst.reg(0)};
        break;
      case KindOfBoolean:
        v << copy{v.cns(!!pcns->m_data.num), dst.reg(0)};
        break;
      case KindOfInt64:
      case KindOfPersistentString:
      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentShape:
      case KindOfPersistentArray:
      case KindOfString:
      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfShape:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
      case KindOfRef:
      case KindOfFunc:
      case KindOfClass:
      case KindOfClsMeth:
        v << copy{v.cns(pcns->m_data.num), dst.reg(0)};
        break;
      case KindOfDouble:
        v << copy{v.cns(pcns->m_data.dbl), dst.reg(0)};
        break;
      case KindOfUninit:
        not_reached();
    }
    v << copy{v.cns(pcns->m_type), dst.reg(1)};
  }
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
tv_rval lookupCnsImpl(StringData* nm) {
  tv_rval cns;

  if (UNLIKELY(rds::s_constants().get() != nullptr)) {
    cns = rds::s_constants()->rval(nm);
  }
  if (!cns) {
    cns = Unit::loadCns(const_cast<StringData*>(nm));
  }
  return cns;
}

Cell lookupCnsHelper(StringData* nm, bool error) {
  auto const cns = lookupCnsImpl(nm);
  if (LIKELY(cns != nullptr)) {
    Cell c1;
    cellDup(*cns, c1);
    return c1;
  }

  // Undefined constants.
  if (error) {
    raise_error("Undefined constant '%s'", nm->data());
  } else {
    raise_notice(Strings::UNDEFINED_CONSTANT, nm->data(), nm->data());
    Cell c1;
    c1.m_data.pstr = const_cast<StringData*>(nm);
    c1.m_type = KindOfPersistentString;
    return c1;
  }
  not_reached();
}

Cell lookupCnsHelperNormal(rds::Handle tv_handle,
                           StringData* nm, bool error) {
  assertx(rds::isNormalHandle(tv_handle));
  if (UNLIKELY(rds::isHandleInit(tv_handle))) {
    auto const tv = rds::handleToPtr<TypedValue, rds::Mode::Normal>(tv_handle);
    if (tv->m_data.pref != nullptr) {
      auto callback = (Native::ConstantCallback)(tv->m_data.pref);
      const Cell* cns = callback().asTypedValue();
      if (LIKELY(cns->m_type != KindOfUninit)) {
        Cell c1;
        cellDup(*cns, c1);
        return c1;
      }
    }
  }
  assertx(!rds::isHandleInit(tv_handle));

  return lookupCnsHelper(nm, error);
}

Cell lookupCnsHelperPersistent(rds::Handle tv_handle,
                               StringData* nm, bool error) {
  assertx(rds::isPersistentHandle(tv_handle));
  auto tv = rds::handleToPtr<TypedValue, rds::Mode::Persistent>(tv_handle);
  assertx(tv->m_type == KindOfUninit);

  // Deferred system constants.
  if (UNLIKELY(tv->m_data.pref != nullptr)) {
    auto callback = (Native::ConstantCallback)(tv->m_data.pref);
    const Cell* cns = callback().asTypedValue();
    if (LIKELY(cns->m_type != KindOfUninit)) {
      Cell c1;
      cellDup(*cns, c1);
      return c1;
    }
  }
  return lookupCnsHelper(nm, error);
}

Cell lookupCnsUHelperNormal(rds::Handle tv_handle,
                            StringData* nm, StringData* fallback,
                            bool error) {
  assertx(rds::isNormalHandle(tv_handle));

  // Lookup qualified name in thread-local constants.
  auto cns = lookupCnsImpl(nm);

  // Try cache handle for unqualified name.
  if (UNLIKELY(!cns && rds::isHandleInit(tv_handle, rds::NormalTag{}))) {
    raise_undefined_const_fallback_notice(nm, fallback);
    cns = rds::handleToPtr<TypedValue, rds::Mode::Normal>(tv_handle);
    assertx(type(cns) != KindOfUninit);
  }

  if (LIKELY(cns != nullptr)) {
    Cell c1;
    cellDup(*cns, c1);
    return c1;
  }

  // Lookup unqualified name in thread-local constants.
  raise_undefined_const_fallback_notice(nm, fallback);
  return lookupCnsHelper(fallback, error);
}

Cell lookupCnsUHelperPersistent(rds::Handle tv_handle,
                                StringData* nm, StringData* fallback,
                                bool error) {
  assertx(rds::isPersistentHandle(tv_handle));

  // Lookup qualified name in thread-local constants.
  auto cns = lookupCnsImpl(nm);

  if (UNLIKELY(!cns)) {
    // Try cache handle for unqualified name.
    auto const tv =
      rds::handleToPtr<TypedValue, rds::Mode::Persistent>(tv_handle);
    if (tv->m_type != KindOfUninit) {
      raise_undefined_const_fallback_notice(nm, fallback);
      cns = tv;
    }
  }

  if (LIKELY(cns != nullptr)) {
    Cell c1;
    cellDup(*cns, c1);
    return c1;
  }

  raise_undefined_const_fallback_notice(nm, fallback);
  return lookupCnsHelper(fallback, error);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void implLookupCns(IRLS& env, const IRInstruction* inst) {
  auto const cnsName = inst->src(0)->strVal();
  auto const ch = makeCnsHandle(cnsName);
  assertx(rds::isHandleBound(ch));

  auto const args = argGroup(env, inst)
    .imm(ch)
    .immPtr(cnsName)
    .imm(inst->is(LookupCnsE));

  cgCallHelper(
    vmain(env), env,
    rds::isNormalHandle(ch)
      ? CallSpec::direct(lookupCnsHelperNormal)
      : CallSpec::direct(lookupCnsHelperPersistent),
    callDestTV(env, inst),
    SyncOptions::Sync,
    args
  );
}

void implLookupCnsU(IRLS& env, const IRInstruction* inst) {
  auto const cnsName = inst->src(0)->strVal();
  auto const fallbackName = inst->src(1)->strVal();

  auto const fallbackCh = makeCnsHandle(fallbackName);
  assertx(rds::isHandleBound(fallbackCh));

  auto const args = argGroup(env, inst)
    .imm(fallbackCh)
    .immPtr(cnsName)
    .immPtr(fallbackName)
    .imm(inst->is(LookupCnsUE));

  cgCallHelper(
    vmain(env), env,
    rds::isNormalHandle(fallbackCh)
      ? CallSpec::direct(lookupCnsUHelperNormal)
      : CallSpec::direct(lookupCnsUHelperPersistent),
    callDestTV(env, inst),
    SyncOptions::Sync,
    args
  );
}

}

void cgLookupCns(IRLS& env, const IRInstruction* inst) {
  implLookupCns(env, inst);
}

void cgLookupCnsE(IRLS& env, const IRInstruction* inst) {
  implLookupCns(env, inst);
}

void cgLookupCnsU(IRLS& env, const IRInstruction* inst) {
  implLookupCnsU(env, inst);
}

void cgLookupCnsUE(IRLS& env, const IRInstruction* inst) {
  implLookupCnsU(env, inst);
}

///////////////////////////////////////////////////////////////////////////////

void cgLdClsCns(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdClsCns>();
  auto const link = rds::bindClassConstant(extra->clsName, extra->cnsName);
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = checkRDSHandleInitialized(v, link.handle());
  fwdJcc(v, env, CC_NE, sf, inst->taken());
  v << lea{rvmtl()[link.handle()], dst};
}

void cgLdSubClsCns(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdSubClsCns>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const slot = extra->slot;
  auto const tmp = v.makeReg();
  v << load{srcLoc(env, inst, 0).reg()[Class::constantsVecOff()], tmp};
  v << lea{tmp[slot * sizeof(Class::Const) + offsetof(Class::Const, val)], dst};
}

void cgCheckSubClsCns(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<CheckSubClsCns>();
  auto& v = vmain(env);

  auto const slot = extra->slot;
  auto const tmp = v.makeReg();
  auto const sf = v.makeReg();
  v << load{srcLoc(env, inst, 0).reg()[Class::constantsVecOff()], tmp};
  emitCmpLowPtr<StringData>(v, sf, v.cns(extra->cnsName),
                            tmp[slot * sizeof(Class::Const) +
                                offsetof(Class::Const, name)]);
  fwdJcc(v, env, CC_NE, sf, inst->taken());
}

void cgLdClsCnsVecLen(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const off = Class::constantsVecLenOff();

  static_assert(
    Class::constantsVecLenSize() == 4,
    "Class::constantsVecLenSize() must be 4 bytes "
    "(if you changed it, fix the following code)");

  v << loadzlq{cls[off], dst};
}

void cgProfileSubClsCns(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ProfileSubClsCns>();

  auto const args = argGroup(env, inst)
    .addr(rvmtl(), safe_cast<int32_t>(extra->handle))
    .ssa(0)
    .imm(extra->cnsName);

  auto const dst = dstLoc(env, inst, 0).reg();

  cgCallHelper(vmain(env), env, CallSpec::method(&ClsCnsProfile::reportClsCns),
               callDest(dst), SyncOptions::None, args);
}


Cell lookupClsCnsHelper(TypedValue* cache, const NamedEntity* ne,
                        const StringData* cls, const StringData* cns) {
  auto const clsCns = g_context->lookupClsCns(ne, cls, cns);
  cellDup(clsCns, *cache);
  return clsCns;
}

void cgInitClsCns(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<InitClsCns>();
  auto const link = rds::bindClassConstant(extra->clsName, extra->cnsName);
  assertx(link.isNormal());
  auto& v = vmain(env);

  auto const args = argGroup(env, inst)
    .addr(rvmtl(), safe_cast<int32_t>(link.handle()))
    .immPtr(NamedEntity::get(extra->clsName))
    .immPtr(extra->clsName)
    .immPtr(extra->cnsName);

  cgCallHelper(v, env, CallSpec::direct(lookupClsCnsHelper),
               callDestTV(env, inst), SyncOptions::Sync, args);

  markRDSHandleInitialized(v, link.handle());
}

void cgLdTypeCns(IRLS& env, const IRInstruction* inst) {
  auto const cns = srcLoc(env, inst, 0).reg();
  auto const ret = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testqi{0x1, cns, sf};
  fwdJcc(v, env, CC_Z, sf, inst->taken());
  v << xorqi{0x1, cns, ret, v.makeReg()};
}

static ArrayData* loadClsTypeCnsHelper(
  const Class* cls, const StringData* name
) {
  auto typeCns = cls->clsCnsGet(name, true);
  if (typeCns.m_type == KindOfUninit) {
    if (cls->hasTypeConstant(name, true)) {
      raise_error("Type constant %s::%s is abstract",
                  cls->name()->data(), name->data());
    } else {
      raise_error("Non-existent type constant %s::%s",
                  cls->name()->data(), name->data());
    }
  }

  assertx(isArrayLikeType(typeCns.m_type));
  assertx(typeCns.m_data.parr->isDictOrDArray());
  assertx(typeCns.m_data.parr->isStatic());
  return typeCns.m_data.parr;
}

void cgLdClsTypeCns(IRLS& env, const IRInstruction* inst) {
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(vmain(env), env, CallSpec::direct(loadClsTypeCnsHelper),
               callDest(env, inst), SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////

}}}
