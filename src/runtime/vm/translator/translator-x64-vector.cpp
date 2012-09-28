/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/complex_types.h>
#include <runtime/vm/translator/asm-x64.h>
#include <runtime/vm/translator/translator-deps.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/member_operations.h>
#include <runtime/vm/stats.h>

#include <runtime/vm/translator/translator-x64-internal.h>

namespace HPHP {
namespace VM {
namespace Transl {

/*
 * Translator for vector instructions.
 */

#define IE(cond, argIf, argElse) \
  ((cond) ? (argIf) : (argElse))
#define PREP_CTX(ctxFixed, pr)                                         \
  Class* ctx = NULL;                                                   \
  LazyScratchReg rCtx(m_regMap);                                       \
  if (ctxFixed) {                                                      \
    ASSERT(isContextFixed());                                          \
    ctx = arGetContextClass(curFrame());                               \
  } else {                                                             \
    rCtx.alloc(pr);                                                    \
    a.  load_reg64_disp_reg64(rsp, offsetof(MInstrState, ctx), *rCtx); \
  }
#define ML(loc, a, regMap, rMis)           \
  IE(loc.isLiteral(),                      \
     _am.addLiteral(loc, a, regMap, rMis), \
     A(loc))

#define PREP_RESULT(useTvR)                                                   \
  if (!useTvR) {                                                              \
    m_regMap.smashLoc(result.location);                                       \
    ScratchReg rScratch(m_regMap);                                            \
    a.  lea_reg64_disp_reg64(rVmSp, vstackOffset(ni, mResultStackOffset(ni)), \
                             *rScratch);                                      \
    emitStoreUninitNull(a, 0, *rScratch);                                     \
  }
#define PREP_CTX(ctxFixed, pr)                                         \
  Class* ctx = NULL;                                                   \
  LazyScratchReg rCtx(m_regMap);                                       \
  if (ctxFixed) {                                                      \
    ASSERT(isContextFixed());                                          \
    ctx = arGetContextClass(curFrame());                               \
  } else {                                                             \
    rCtx.alloc(pr);                                                    \
    a.  load_reg64_disp_reg64(rsp, offsetof(MInstrState, ctx), *rCtx); \
  }
#define CTX(ctxFixed)     \
  IE((ctxFixed),          \
     IMM(uintptr_t(ctx)), \
     R(*rCtx))
#define PREP_VAL(useRVal, pr)                                   \
  LazyScratchReg rVal(m_regMap);                                \
  if (useRVal) {                                                \
    rVal.alloc(pr);                                             \
    PhysReg pr;                                                 \
    int disp;                                                   \
    locToRegDisp(val.location, &pr, &disp);                     \
    a.  load_reg64_disp_reg64(pr, disp + TVOFF(m_data), *rVal); \
  }
#define VAL(useRVal)  \
  IE((useRVal),       \
     R(*rVal),        \
     A(val.location))

#define RESULT(useTvR)                            \
  IE((useTvR),                                    \
     RPLUS(rsp, offsetof(MInstrState, tvResult)), \
     A(result.location))

#define TRANSLATE_MINSTR_GENERIC(instr, t, ni) do {                        \
  ASSERT(RuntimeOption::EvalJitMGeneric);                             \
  SKTRACE(2, ni.source, "%s\n", __func__);                            \
  const MInstrInfo& mii = getMInstrInfo(Op##instr##M);                \
  bool ctxFixed;                                                      \
  unsigned mInd, iInd;                                                \
  PhysReg rBase;                                                      \
  emitMPre((t), (ni), mii, ctxFixed, mInd, iInd, rBase);              \
  emitFinal##instr##MOp((t), (ni), mii, ctxFixed, mInd, iInd, rBase); \
  emitMPost((t), (ni), mii);                                          \
} while (0)


int TranslatorX64::mResultStackOffset(const NormalizedInstruction& ni) const {
  int stackDest = 0 - int(sizeof(Cell));
  for (unsigned i = 0; i < ni.inputs.size(); ++i) {
    const DynLocation& input = *ni.inputs[i];
    if (input.location.isStack()) {
      stackDest += int(sizeof(Cell));
    }
  }
  return stackDest;
}

bool TranslatorX64::generateMVal(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii) const {
  if (mii.valCount() == 1) {
    const DynLocation& input = *ni.inputs[0];
    const DynLocation& output = *ni.outStack;
    // Some instruction sequences, e.g. CGetL..SetM..PopC are optimized during
    // analysis to avoid the push/pop, in which case the input is a local
    // instead of a stack, and no val results from executing the VM instruction.
    if (input.isStack() && &output != NULL) {
      return true;
    }
  } else if (mii.instr() == MI_IncDecM) {
    const DynLocation& output = *ni.outStack;
    if (&output != NULL) {
      return true;
    }
  }
  return false;
}

bool TranslatorX64::logicalTeleportMVal(const Tracelet& t,
                                        const NormalizedInstruction& ni,
                                        const MInstrInfo& mii) const {
  return ((mii.instr() == MI_SetM || mii.instr() == MI_BindM)
          && generateMVal(t, ni, mii));
}

bool TranslatorX64::teleportMVal(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii) const {
  if (logicalTeleportMVal(t, ni, mii)) {
    const DynLocation& input = *ni.inputs[0];
    const DynLocation& output = *ni.outStack;
    // No teleport is required if the only stack input is the result, because
    // the result ends up in the same place as the input.
    if (input.location != output.location) {
      return true;
    }
  }
  return false;
}

bool TranslatorX64::useTvResult(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                const MInstrInfo& mii) const {
  // Some of the *M instructions store a result to the VM stack, which may be
  // at the same location as an input.  If an input and an output are overlaid,
  // it is possible to write the result directly to its final location if the
  // overlaid input is not refcounted (won't be decref'ed by emitMPost()).
  // Otherwise the result must be temporarily stored to tvResult, then copied
  // to its final location after inputs have been discarded.
  const DynLocation& output = *ni.outStack;
  if (&output == NULL) {
    SKTRACE(2, ni.source, "%s (no stack output) --> false\n", __func__);
    return false;
  }
  ASSERT(output.location.isStack());
  if (mii.instr() == MI_SetM || mii.instr() == MI_BindM) {
    SKTRACE(2, ni.source, "%s (%s val) --> false\n",
            __func__, mii.instr() == MI_SetM ? "SetM" : "BindM");
    return false;
  }
  for (unsigned i = mii.valCount(); i < ni.inputs.size(); ++i) {
    const DynLocation& input = *ni.inputs[i];
    if (input.location.isStack()) {
      // Only the bottommost stack input can possibly overlay the output.
      ASSERT(input.location == output.location);
      SKTRACE(2, ni.source, "%s input %u/[0..%u) --> %s\n",
              __func__, i, ni.inputs.size(),
              (IS_REFCOUNTED_TYPE(input.outerType())
              || i == ni.inputs.size() - 1) ? "true" : "false");
      // Use tvResult if the overlaid input is refcounted, or if it is the last
      // input (in which case it may still be in use at the time the result is
      // written).
      return IS_REFCOUNTED_TYPE(input.outerType())
             || i == ni.inputs.size() - 1;
    }
  }
  // No stack inputs.  If we were to write the result directly to the VM stack,
  // it would potentially be clobbered by reentry, e.g. for ArrayAccess-related
  // destruction.
  SKTRACE(2, ni.source, "%s (no stack inputs) --> true\n", __func__);
  return true;
}

void TranslatorX64::emitBaseLCR(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                const MInstrInfo& mii, unsigned iInd,
                                PhysReg& rBase) {
  LocationCode lCode = ni.immVec.locationCode();
  const MInstrAttr& mia = mii.getAttr(lCode);
  SKTRACE(2, ni.source, "%s %#lx %s%s\n", __func__, long(a.code.frontier),
          (mia & MIA_warn) ? "W" : "", (mia & MIA_define) ? "D" : "");
  const DynLocation& base = *ni.inputs[iInd];
  if (mia & MIA_warn) {
    if (base.rtt.isUninit()) {
      const StringData* name = local_name(base.location);
      EMIT_RCALL(a, ni, raiseUndefVariable, IMM((uintptr_t)name));
    }
  }
  if (mia & MIA_define) {
    if (base.rtt.isUninit()) {
      emitStoreNull(a, base.location);
    }
  }
  m_regMap.smashLoc(base.location);
  PhysReg pr;
  int disp;
  locToRegDisp(base.location, &pr, &disp);
  rBase = m_regMap.allocScratchReg();
  if (base.isVariant()) {
    // Get inner value.
    a.  load_reg64_disp_reg64(pr, disp + TVOFF(m_data), rBase);
  } else {
    a.  lea_reg64_disp_reg64(pr, disp, rBase);
  }
}

template <bool warn, bool define>
static inline TypedValue* baseNImpl(TypedValue* key,
                                    TranslatorX64::MInstrState* mis,
                                    ActRec* fp) {
  TypedValue* base;
  StringData* name = prepareKey(key).detach();
  const Func* func = fp->m_func;
  Id id = func->lookupVarId(name);
  if (id != kInvalidId) {
    base = frame_local(fp, id);
  } else {
    ASSERT(!fp->hasInvName());
    if (define) {
      if (fp->m_varEnv == NULL) {
        fp->m_varEnv = VarEnv::createLazyAttach(fp);
      }
      base = fp->m_varEnv->lookup(name);
      if (base == NULL) {
        if (warn) {
          raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
        }
        TypedValue tv;
        TV_WRITE_NULL(&tv);
        fp->m_varEnv->set(name, &tv);
        base = fp->m_varEnv->lookup(name);
      }
    } else {
      if (fp->m_varEnv == NULL || (base = fp->m_varEnv->lookup(name)) == NULL) {
        if (warn) {
          raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
        }
        TV_WRITE_NULL(&mis->tvScratch);
        base = &mis->tvScratch;
      }
    }
  }
  LITSTR_DECREF(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

HOT_FUNC_VM
static TypedValue* baseN(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<false, false>(key, mis, (ActRec*)rbp->m_savedRbp);
}

HOT_FUNC_VM
static TypedValue* baseNW(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<true, false>(key, mis, (ActRec*)rbp->m_savedRbp);
}

HOT_FUNC_VM
static TypedValue* baseND(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<false, true>(key, mis, (ActRec*)rbp->m_savedRbp);
}

HOT_FUNC_VM
static TypedValue* baseNWD(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<true, true>(key, mis, (ActRec*)rbp->m_savedRbp);
}

void TranslatorX64::emitBaseN(const Tracelet& t,
                              const NormalizedInstruction& ni,
                              const MInstrInfo& mii, unsigned iInd,
                              PhysReg& rBase) {
  LocationCode lCode = ni.immVec.locationCode();
  const MInstrAttr& mia = mii.getAttr(lCode);
  SKTRACE(2, ni.source, "%s %#lx %s%s\n",
          __func__, long(a.code.frontier),
          (mia & MIA_warn) ? "W" : "", (mia & MIA_define) ? "D" : "");
  typedef TypedValue* (*BaseNOp)(TypedValue*, TranslatorX64::MInstrState*);
  ASSERT(MIA_warn == 0x1 && MIA_define == 0x2);
  static const BaseNOp baseNOps[] = {baseN, baseNW, baseND, baseNWD};
  ASSERT((mia & MIA_base) < sizeof(baseNOps)/sizeof(BaseNOp));
  BaseNOp baseNOp = baseNOps[mia & MIA_base];
  const DynLocation& base = *ni.inputs[iInd];
  m_regMap.smashLoc(base.location);
  EMIT_RCALL(a, ni, baseNOp, A(base.location), R(rsp));
  rBase = m_regMap.allocScratchReg(rax);
}

template <bool warn, bool define>
static inline TypedValue* baseGImpl(TypedValue* key,
                                    TranslatorX64::MInstrState* mis) {
  TypedValue* base;
  StringData* name = prepareKey(key).detach();
  VarEnv* varEnv = g_vmContext->m_globalVarEnv;
  ASSERT(varEnv != NULL);
  base = varEnv->lookup(name);
  if (base == NULL) {
    if (warn) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    if (define) {
      TypedValue tv;
      TV_WRITE_NULL(&tv);
      varEnv->set(name, &tv);
      base = varEnv->lookup(name);
    } else {
      TV_WRITE_NULL(&mis->tvScratch);
      base = &mis->tvScratch;
    }
  }
  LITSTR_DECREF(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

HOT_FUNC_VM
static TypedValue* baseG(TypedValue* key, TranslatorX64::MInstrState* mis) {
  return baseGImpl<false, false>(key, mis);
}

HOT_FUNC_VM
static TypedValue* baseGW(TypedValue* key, TranslatorX64::MInstrState* mis) {
  return baseGImpl<true, false>(key, mis);
}

HOT_FUNC_VM
static TypedValue* baseGD(TypedValue* key, TranslatorX64::MInstrState* mis) {
  return baseGImpl<false, true>(key, mis);
}

HOT_FUNC_VM
static TypedValue* baseGWD(TypedValue* key, TranslatorX64::MInstrState* mis) {
  return baseGImpl<true, true>(key, mis);
}

void TranslatorX64::emitBaseG(const Tracelet& t,
                              const NormalizedInstruction& ni,
                              const MInstrInfo& mii, unsigned iInd,
                              PhysReg& rBase) {
  LocationCode lCode = ni.immVec.locationCode();
  const MInstrAttr& mia = mii.getAttr(lCode);
  SKTRACE(2, ni.source, "%s %#lx %s%s\n", __func__, long(a.code.frontier),
          (mia & MIA_warn) ? "W" : "", (mia & MIA_define) ? "D" : "");
  typedef TypedValue* (*BaseGOp)(TypedValue*, TranslatorX64::MInstrState*);
  ASSERT(MIA_warn == 0x1 && MIA_define == 0x2);
  static const BaseGOp baseGOps[] = {baseG, baseGW, baseGD, baseGWD};
  ASSERT((mia & MIA_base) < sizeof(baseGOps)/sizeof(BaseGOp));
  BaseGOp baseGOp = baseGOps[mia & MIA_base];
  const DynLocation& base = *ni.inputs[iInd];
  m_regMap.smashLoc(base.location);
  EMIT_RCALL(a, ni, baseGOp, A(base.location), R(rsp));
  rBase = m_regMap.allocScratchReg(rax);
}

HOT_FUNC_VM
static TypedValue* baseS(Class* ctx, TypedValue* key, const Class* cls,
                         TranslatorX64::MInstrState* mis) {
  TypedValue* base;
  StringData* name = prepareKey(key).detach();
  bool visible, accessible;
  base = cls->getSProp(ctx, name, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(), name->data());
  }
  LITSTR_DECREF(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

HOT_FUNC_VM
static TypedValue* baseSClsRef(Class* ctx, TypedValue* key,
                               TypedValue* clsRef,
                               TranslatorX64::MInstrState* mis) {
  ASSERT(clsRef->m_type == KindOfClass);
  const Class* cls = clsRef->m_data.pcls;
  return baseS(ctx, key, cls, mis);
}

void TranslatorX64::emitBaseS(const Tracelet& t,
                              const NormalizedInstruction& ni, unsigned iInd,
                              bool ctxFixed, PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx\n", __func__, long(a.code.frontier));
  const DynLocation& key = *ni.inputs[iInd];
  m_regMap.smashLoc(key.location);
  const DynLocation& clsRef = *ni.inputs[ni.inputs.size()-1];
  ASSERT(clsRef.valueType() == KindOfClass);
  const Class* cls = clsRef.rtt.valueClass();
  PREP_CTX(ctxFixed, rdi);
  // Emit the appropriate helper call.
  if (cls != NULL) {
    EMIT_RCALL(a, ni, baseS,
                      CTX(ctxFixed),
                      A(key.location),
                      IMM(uintptr_t(cls)),
                      R(rsp));
  } else {
    m_regMap.smashLoc(clsRef.location);
    EMIT_RCALL(a, ni, baseSClsRef,
                      CTX(ctxFixed),
                      A(key.location),
                      A(clsRef.location),
                      R(rsp));
  }
  rBase = m_regMap.allocScratchReg(rax);
}

void TranslatorX64::emitBaseOp(const Tracelet& t,
                               const NormalizedInstruction& ni,
                               const MInstrInfo& mii, unsigned iInd,
                               bool ctxFixed, PhysReg& rBase) {
  LocationCode lCode = ni.immVec.locationCode();
  switch (lCode) {
  case LL: case LC: case LR: emitBaseLCR(t, ni, mii, iInd, rBase);    break;
  case LGL: case LGC:        emitBaseG(t, ni, mii, iInd, rBase);      break;
  case LNL: case LNC:        emitBaseN(t, ni, mii, iInd, rBase);      break;
  case LSL: case LSC:        emitBaseS(t, ni, iInd, ctxFixed, rBase); break;
  default:                   not_reached();
  }
}

template <bool unboxKey, bool warn, bool define, bool reffy, bool unset>
static inline TypedValue* elemImpl(TypedValue* base, TypedValue* key,
                                   TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  if (unset) {
    return ElemU(mis->tvScratch, mis->tvRef, base, key);
  } else if (define) {
    return ElemD<warn, reffy>(mis->tvScratch, mis->tvRef, base, key);
  } else {
    return Elem<warn>(mis->tvScratch, mis->tvRef, base, mis->baseStrOff, key);
  }
}

static TypedValue* elemX(TypedValue* base, TypedValue* key,
                         TranslatorX64::MInstrState* mis) {
  not_reached();
}

HOT_FUNC_VM
static TypedValue* elemCU(TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return elemImpl<false, false, false, false, true>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemCWDR(TypedValue* base, TypedValue* key,
                            TranslatorX64::MInstrState* mis) {
  return elemImpl<false, true, true, true, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemCDR(TypedValue* base, TypedValue* key,
                           TranslatorX64::MInstrState* mis) {
  return elemImpl<false, false, true, true, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemCWD(TypedValue* base, TypedValue* key,
                           TranslatorX64::MInstrState* mis) {
  return elemImpl<false, true, true, false, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemCD(TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return elemImpl<false, false, true, false, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemCW(TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return elemImpl<false, true, false, false, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemC(TypedValue* base, TypedValue* key,
                         TranslatorX64::MInstrState* mis) {
  return elemImpl<false, false, false, false, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemLU(TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return elemImpl<true, false, false, false, true>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemLWDR(TypedValue* base, TypedValue* key,
                            TranslatorX64::MInstrState* mis) {
  return elemImpl<true, true, true, true, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemLDR(TypedValue* base, TypedValue* key,
                           TranslatorX64::MInstrState* mis) {
  return elemImpl<true, false, true, true, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemLWD(TypedValue* base, TypedValue* key,
                           TranslatorX64::MInstrState* mis) {
  return elemImpl<true, true, true, false, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemLD(TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return elemImpl<true, false, true, false, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemLW(TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return elemImpl<true, true, false, false, false>(base, key, mis);
}

HOT_FUNC_VM
static TypedValue* elemL(TypedValue* base, TypedValue* key,
                         TranslatorX64::MInstrState* mis) {
  return elemImpl<true, false, false, false, false>(base, key, mis);
}

void TranslatorX64::emitElem(const Tracelet& t,
                             const NormalizedInstruction& ni,
                             const MInstrInfo& mii, unsigned mInd,
                             unsigned iInd, PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  MemberCode mCode = ni.immVecM[mInd];
  const MInstrAttr& mia = mii.getAttr(mCode);
  const DynLocation& memb = *ni.inputs[iInd];
  m_regMap.smashLoc(memb.location);
  typedef TypedValue* (*ElemOp)(TypedValue*, TypedValue*,
                                TranslatorX64::MInstrState*);
  ASSERT(MIA_warn == 0x1 && MIA_define == 0x2 && MIA_reffy == 0x4 &&
         MIA_unset == 0x8);
  static const ElemOp localElemOps[]
    = {elemL,  elemLW, elemLD, elemLWD, elemX, elemX, elemLDR, elemLWDR,
       elemLU, elemX,  elemX,  elemX,   elemX, elemX, elemX,   elemX};
  static const ElemOp cellElemOps[]
    = {elemC,  elemCW, elemCD, elemCWD, elemX, elemX, elemCDR, elemCWDR,
       elemCU, elemX,  elemX,  elemX,   elemX, elemX, elemX,   elemX};
  ASSERT((mia & MIA_intermediate) < sizeof(localElemOps)/sizeof(ElemOp));
  ASSERT((mia & MIA_intermediate) < sizeof(cellElemOps)/sizeof(ElemOp));
  ElemOp elemOp = ((mCode == MEL) ? localElemOps : cellElemOps)
                  [mia & MIA_intermediate];
  ASSERT(elemOp != elemX);
  EMIT_RCALL(a, ni, elemOp, R(rBase), ML(memb.location, a, m_regMap, rsp),
             R(rsp));
  rBase = m_regMap.allocScratchReg(rax);
}

template <bool unboxKey, bool warn, bool define, bool unset>
static inline TypedValue* propImpl(Class* ctx, TypedValue* base,
                                   TypedValue* key,
                                   TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  return Prop<warn, define, unset>(mis->tvScratch, mis->tvRef, ctx, base, key);
}

static TypedValue* propX(Class* ctx, TypedValue* base, TypedValue* key,
                         TranslatorX64::MInstrState* mis) {
  not_reached();
}

HOT_FUNC_VM
static TypedValue* propCU(Class* ctx, TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return propImpl<false, false, false, true>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propCWD(Class* ctx, TypedValue* base, TypedValue* key,
                           TranslatorX64::MInstrState* mis) {
  return propImpl<false, true, true, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propCW(Class* ctx, TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return propImpl<false, true, false, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propCD(Class* ctx, TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return propImpl<false, false, true, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propC(Class* ctx, TypedValue* base, TypedValue* key,
                         TranslatorX64::MInstrState* mis) {
  return propImpl<false, false, false, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propLU(Class* ctx, TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return propImpl<true, false, false, true>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propLWD(Class* ctx, TypedValue* base, TypedValue* key,
                           TranslatorX64::MInstrState* mis) {
  return propImpl<true, true, true, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propLW(Class* ctx, TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return propImpl<true, true, false, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propLD(Class* ctx, TypedValue* base, TypedValue* key,
                          TranslatorX64::MInstrState* mis) {
  return propImpl<true, false, true, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static TypedValue* propL(Class* ctx, TypedValue* base, TypedValue* key,
                         TranslatorX64::MInstrState* mis) {
  return propImpl<true, false, false, false>(ctx, base, key, mis);
}

void TranslatorX64::emitProp(const Tracelet& t,
                             const NormalizedInstruction& ni,
                             const MInstrInfo& mii, bool ctxFixed,
                             unsigned mInd, unsigned iInd,
                             PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  MemberCode mCode = ni.immVecM[mInd];
  const MInstrAttr& mia = mii.getAttr(mCode);
  const DynLocation& memb = *ni.inputs[iInd];
  m_regMap.smashLoc(memb.location);
  typedef TypedValue* (*PropOp)(Class*, TypedValue*, TypedValue*,
                                TranslatorX64::MInstrState*);
  ASSERT(MIA_warn == 0x1 && MIA_define == 0x2 && MIA_unset == 0x8);
  static const PropOp localPropOps[]
    = {propL,  propLW, propLD, propLWD, propX, propX, propLD, propLWD,
       propLU, propX,  propX,  propX,   propX, propX, propX,  propX};
  static const PropOp cellPropOps[]
    = {propC,  propCW, propCD, propCWD, propX, propX, propCD, propCWD,
       propCU, propX,  propX,  propX,   propX, propX, propX,  propX};
  ASSERT((mia & MIA_intermediate) < sizeof(localPropOps)/sizeof(PropOp));
  ASSERT((mia & MIA_intermediate) < sizeof(cellPropOps)/sizeof(PropOp));
  PropOp propOp = ((mCode == MPL) ? localPropOps : cellPropOps)
                  [mia & MIA_intermediate];
  ASSERT(propOp != propX);
  PREP_CTX(ctxFixed, rdi);
  // Emit the appropriate helper call.
  EMIT_RCALL(a, ni, propOp,
                    CTX(ctxFixed),
                    R(rBase),
                    ML(memb.location, a, m_regMap, rsp),
                    R(rsp));
  rBase = m_regMap.allocScratchReg(rax);
}

HOT_FUNC_VM
static TypedValue* newElem(TypedValue* base, TranslatorX64::MInstrState* mis) {
  return NewElem(mis->tvScratch, mis->tvRef, base);
}

void TranslatorX64::emitNewElem(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                unsigned mInd, PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u\n",
          __func__, long(a.code.frontier), mInd);
  EMIT_RCALL(a, ni, newElem, R(rBase), R(rsp));
  rBase = m_regMap.allocScratchReg(rax);
}

void TranslatorX64::emitIntermediateOp(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii, bool ctxFixed,
                                       unsigned mInd, unsigned& iInd,
                                       PhysReg& rBase) {
  switch (ni.immVecM[mInd]) {
  case MEC: case MEL: case MET: case MEI:
    emitElem(t, ni, mii, mInd, iInd, rBase);
    ++iInd;
    break;
  case MPC: case MPL: case MPT:
    emitProp(t, ni, mii, ctxFixed, mInd, iInd, rBase);
    ++iInd;
    break;
  case MW:
    ASSERT(mii.newElem());
    emitNewElem(t, ni, mInd, rBase);
    break;
  default: not_reached();
  }
}

bool TranslatorX64::needFirstRatchet(const Tracelet& t,
                                     const NormalizedInstruction& ni,
                                     const MInstrInfo& mii) const {
  if (ni.inputs[mii.valCount()]->valueType() == KindOfArray) {
    switch (ni.immVecM[0]) {
    case MEC: case MEL: case MET: case MEI: return false;
    case MPC: case MPL: case MPT: case MW:  return true;
    default: not_reached();
    }
  }
  return true;
}

bool TranslatorX64::needFinalRatchet(const Tracelet& t,
                                     const NormalizedInstruction& ni,
                                     const MInstrInfo& mii) const {
  return mii.finalGet();
}

unsigned TranslatorX64::nLogicalRatchets(const Tracelet& t,
                                         const NormalizedInstruction& ni,
                                         const MInstrInfo& mii) const {
  // Ratchet operations occur after each intermediate operation, except
  // possibly the first and last (see need{First,Final}Ratchet()).  No actual
  // ratchet occurs after the final operation, but this means that both tvRef
  // and tvRef2 can contain references just after the final operation.  Here we
  // pretend that a ratchet occurs after the final operation, i.e. a "logical"
  // ratchet.  The reason for counting logical ratchets as part of the total is
  // the following case, in which the logical count is 0:
  //
  //   (base is array)
  //   BaseL
  //   IssetElemL
  //     no logical ratchet
  //
  // Following are a few more examples to make the algorithm clear:
  //
  //   (base is array)      (base is object)   (base is object)
  //   BaseL                BaseL              BaseL
  //   ElemL                ElemL              CGetPropL
  //     no ratchet           ratchet            logical ratchet
  //   ElemL                PropL
  //     ratchet              ratchet
  //   ElemL                CGetElemL
  //     ratchet              logical ratchet
  //   IssetElemL
  //     logical ratchet
  //
  //   (base is array)
  //   BaseL
  //   ElemL
  //     no ratchet
  //   ElemL
  //     ratchet
  //   ElemL
  //     logical ratchet
  //   SetElemL
  //     no ratchet
  return ni.immVecM.size()
         - (needFirstRatchet(t, ni, mii) ? 0 : 1)
         - (needFinalRatchet(t, ni, mii) ? 0 : 1);
}

int TranslatorX64::ratchetInd(const Tracelet& t,
                              const NormalizedInstruction& ni,
                              const MInstrInfo& mii,
                              unsigned mInd) const {
  return needFirstRatchet(t, ni, mii) ? int(mInd) : int(mInd) - 1;
}

void TranslatorX64::emitRatchetRefs(const Tracelet& t,
                                    const NormalizedInstruction& ni,
                                    const MInstrInfo& mii,
                                    unsigned mInd, const PhysReg& rBase) {
  if (ratchetInd(t, ni, mii, mInd) < 0
      || ratchetInd(t, ni, mii, mInd) >= int(nLogicalRatchets(t, ni, mii))) {
    return;
  }
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, ratchetInd=%d/[0..%u)\n",
          __func__, long(a.code.frontier), mInd, ratchetInd(t, ni, mii, mInd),
          nLogicalRatchets(t, ni, mii));
  {
    UnlessUninit uu(a, rsp, offsetof(MInstrState, tvRef));
    // Clean up tvRef2 before overwriting it.
    if (ratchetInd(t, ni, mii, mInd) > 0) {
      emitDecRefGeneric(ni, rsp, offsetof(MInstrState, tvRef2));
    }
    // Copy tvRef to tvRef2.
    {
      ScratchReg rScratch(m_regMap);
      ASSERT(sizeof(TypedValue) % 8 == 0);
      for (size_t off = 0; off < sizeof(TypedValue); off += 8) {
        a.load_reg64_disp_reg64(rsp, offsetof(MInstrState, tvRef) + off,
                                *rScratch);
        a.store_reg64_disp_reg64(*rScratch,
                                 offsetof(MInstrState, tvRef2) + off, rsp);
      }
    }
    // Reset tvRef.
    emitStoreUninitNull(a, offsetof(MInstrState, tvRef), rsp);
    // Adjust base pointer.
    a.    lea_reg64_disp_reg64(rsp, offsetof(MInstrState, tvRef2), rBase);
  }
}
template <bool unboxKey>
static inline void cGetPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  base = Prop<true, false, false>(*result, mis->tvRef, ctx, base, key);
  if (base != result) {
    // Save a copy of the result.
    tvDup(base, result);
  }
  if (result->m_type == KindOfRef) {
    tvUnbox(result);
  }
}

HOT_FUNC_VM
static void cGetPropL(Class* ctx, TypedValue* base, TypedValue* key,
                      TypedValue* result, TranslatorX64::MInstrState* mis) {
  cGetPropImpl<true>(ctx, base, key, result, mis);
}

HOT_FUNC_VM
static void cGetPropC(Class* ctx, TypedValue* base, TypedValue* key,
                      TypedValue* result, TranslatorX64::MInstrState* mis) {
  cGetPropImpl<false>(ctx, base, key, result, mis);
}

void TranslatorX64::emitCGetProp(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, bool ctxFixed,
                                 unsigned mInd, unsigned iInd,
                                 const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& memb = *ni.inputs[iInd];
  void (*cGetPropOp)(Class*, TypedValue*, TypedValue*, TypedValue*,
                     MInstrState*) =
    ni.immVecM[mInd] == MPL ? cGetPropL : cGetPropC;
  m_regMap.smashLoc(memb.location);
  const DynLocation& result = *ni.outStack;
  PREP_CTX(ctxFixed, rdi);
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  // Emit the appropriate helper call.
  EMIT_RCALL(a, ni, cGetPropOp,
                    CTX(ctxFixed),
                    R(rBase),
                    ML(memb.location, a, m_regMap, rsp),
                    RESULT(useTvR),
                    R(rsp));
}

template <bool unboxKey>
static inline void vGetElemImpl(TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  base = ElemD<false, true>(mis->tvScratch, mis->tvRef, base, key);
  if (base == &mis->tvScratch && base->m_type == KindOfUninit) {
    // Error (no result was set).
    tvWriteNull(result);
    tvBox(result);
  } else {
    if (base->m_type != KindOfRef) {
      tvBox(base);
    }
    tvDupVar(base, result);
  }
}

HOT_FUNC_VM
static void vGetElemL(TypedValue* base, TypedValue* key, TypedValue* result,
                      TranslatorX64::MInstrState* mis) {
  vGetElemImpl<true>(base, key, result, mis);
}

HOT_FUNC_VM
static void vGetElemC(TypedValue* base, TypedValue* key, TypedValue* result,
                      TranslatorX64::MInstrState* mis) {
  vGetElemImpl<false>(base, key, result, mis);
}

void TranslatorX64::emitVGetElem(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, unsigned mInd,
                                 unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& memb = *ni.inputs[iInd];
  void (*vGetElemOp)(TypedValue*, TypedValue*, TypedValue*, MInstrState*) =
    ni.immVecM[mInd] == MEL ? vGetElemL : vGetElemC;
  m_regMap.smashLoc(memb.location);
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  EMIT_RCALL(a, ni, vGetElemOp,
                    R(rBase),
                    ML(memb.location, a, m_regMap, rsp),
                    RESULT(useTvR),
                    R(rsp));
}

template <bool unboxKey>
static inline void vGetPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  base = Prop<false, true, false>(mis->tvScratch, mis->tvRef, ctx, base, key);
  if (base == &mis->tvScratch && base->m_type == KindOfUninit) {
    // Error (no result was set).
    tvWriteNull(result);
    tvBox(result);
  } else {
    if (base->m_type != KindOfRef) {
      tvBox(base);
    }
    tvDupVar(base, result);
  }
}

HOT_FUNC_VM
static inline void vGetPropL(Class* ctx, TypedValue* base, TypedValue* key,
                            TypedValue* result,
                            TranslatorX64::MInstrState* mis) {
  vGetPropImpl<true>(ctx, base, key, result, mis);
}

HOT_FUNC_VM
static inline void vGetPropC(Class* ctx, TypedValue* base, TypedValue* key,
                            TypedValue* result,
                            TranslatorX64::MInstrState* mis) {
  vGetPropImpl<false>(ctx, base, key, result, mis);
}

void TranslatorX64::emitVGetProp(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, bool ctxFixed,
                                 unsigned mInd, unsigned iInd,
                                 const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& memb = *ni.inputs[iInd];
  void (*vGetPropOp)(Class*, TypedValue*, TypedValue*, TypedValue*,
                     MInstrState*) =
    ni.immVecM[mInd] == MPL ? vGetPropL : vGetPropC;
  m_regMap.smashLoc(memb.location);
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  PREP_CTX(ctxFixed, rdi);
  // Emit the appropriate helper call.
  EMIT_RCALL(a, ni, vGetPropOp,
                    CTX(ctxFixed),
                    R(rBase),
                    ML(memb.location, a, m_regMap, rsp),
                    RESULT(useTvR),
                    R(rsp));
}

template <bool unboxKey, bool useEmpty>
static inline bool issetEmptyElemImpl(TypedValue* base, TypedValue* key,
                                      TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  return IssetEmptyElem<useEmpty>(mis->tvScratch, mis->tvRef, base,
                                  mis->baseStrOff, key);
}

HOT_FUNC_VM
static bool issetElemL(TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyElemImpl<true, false>(base, key, mis);
}

HOT_FUNC_VM
static bool issetElemC(TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyElemImpl<false, false>(base, key, mis);
}

HOT_FUNC_VM
static bool emptyElemL(TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyElemImpl<true, true>(base, key, mis);
}

HOT_FUNC_VM
static bool emptyElemC(TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyElemImpl<false, true>(base, key, mis);
}

template <bool useEmpty>
void TranslatorX64::emitIssetEmptyElem(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii, unsigned mInd,
                                       unsigned iInd, const PhysReg& rBase) {
  const DynLocation& memb = *ni.inputs[iInd];
  bool (*issetEmptyElemOp)(TypedValue*, TypedValue*, MInstrState*) =
    useEmpty ? (ni.immVecM[mInd] == MEL ? emptyElemL : emptyElemC)
             : (ni.immVecM[mInd] == MEL ? issetElemL : issetElemC);
  m_regMap.smashLoc(memb.location);
  EMIT_RCALL(a, ni, issetEmptyElemOp, R(rBase),
             ML(memb.location, a, m_regMap, rsp), R(rsp));
  a.   and_imm32_reg64(1, rax); // Mask garbage bits.
  PhysReg rIssetEmpty = m_regMap.allocScratchReg(rax);
  if (useTvResult(t, ni, mii)) {
    emitStoreTypedValue(a, KindOfBoolean, rIssetEmpty,
                        offsetof(MInstrState, tvResult), rsp);
  } else {
    m_regMap.allocOutputRegs(ni);
    a.  mov_reg64_reg64(rIssetEmpty, getReg(ni.outStack->location));
  }
}

void TranslatorX64::emitIssetElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyElem<false>(t, ni, mii, mInd, iInd, rBase);
}

template <bool unboxKey, bool useEmpty>
static inline bool issetEmptyPropImpl(Class* ctx, TypedValue* base,
                                      TypedValue* key,
                                      TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  return IssetEmptyProp<useEmpty>(ctx, base, key);
}

HOT_FUNC_VM
static bool issetPropL(Class* ctx, TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyPropImpl<true, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static bool issetPropC(Class* ctx, TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyPropImpl<false, false>(ctx, base, key, mis);
}

HOT_FUNC_VM
static bool emptyPropL(Class* ctx, TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyPropImpl<true, true>(ctx, base, key, mis);
}

HOT_FUNC_VM
static bool emptyPropC(Class* ctx, TypedValue* base, TypedValue* key,
                       TranslatorX64::MInstrState* mis) {
  return issetEmptyPropImpl<false, true>(ctx, base, key, mis);
}

template <bool useEmpty>
void TranslatorX64::emitIssetEmptyProp(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii, bool ctxFixed,
                                       unsigned mInd, unsigned iInd,
                                       const PhysReg& rBase) {
  const DynLocation& memb = *ni.inputs[iInd];
  bool (*issetEmptyPropOp)(Class*, TypedValue*, TypedValue*, MInstrState*) =
    useEmpty ? (ni.immVecM[mInd] == MPL ? emptyPropL : emptyPropC)
             : (ni.immVecM[mInd] == MPL ? issetPropL : issetPropC);
  m_regMap.smashLoc(memb.location);
  PREP_CTX(ctxFixed, rdi);
  // Emit the appropriate helper call.
  EMIT_RCALL(a, ni, issetEmptyPropOp,
                    CTX(ctxFixed),
                    R(rBase),
                    ML(memb.location, a, m_regMap, rsp));
  a.   and_imm32_reg64(1, rax); // Mask garbage bits.
  PhysReg rIssetEmpty = m_regMap.allocScratchReg(rax);
  if (useTvResult(t, ni, mii)) {
    emitStoreTypedValue(a, KindOfBoolean, rIssetEmpty,
                        offsetof(MInstrState, tvResult), rsp);
  } else {
    m_regMap.allocOutputRegs(ni);
    a.  mov_reg64_reg64(rIssetEmpty, getReg(ni.outStack->location));
  }
}

void TranslatorX64::emitIssetProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, bool ctxFixed,
                                  unsigned mInd, unsigned iInd,
                                  const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyProp<false>(t, ni, mii, ctxFixed, mInd, iInd, rBase);
}

void TranslatorX64::emitEmptyElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyElem<true>(t, ni, mii, mInd, iInd, rBase);
}

void TranslatorX64::emitEmptyProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, bool ctxFixed,
                                  unsigned mInd, unsigned iInd,
                                  const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyProp<true>(t, ni, mii, ctxFixed, mInd, iInd, rBase);
}

template <bool unboxKey, bool setResult>
static inline void setElemImpl(TypedValue* base, TypedValue* key, Cell* val) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  SetElem<setResult>(base, key, val);
}

HOT_FUNC_VM
static void setElemLR(TypedValue* base, TypedValue* key, Cell* val) {
  setElemImpl<true, true>(base, key, val);
}

HOT_FUNC_VM
static void setElemL(TypedValue* base, TypedValue* key, Cell* val) {
  setElemImpl<true, false>(base, key, val);
}

HOT_FUNC_VM
static void setElemCR(TypedValue* base, TypedValue* key, Cell* val) {
  setElemImpl<false, true>(base, key, val);
}

HOT_FUNC_VM
static void setElemC(TypedValue* base, TypedValue* key, Cell* val) {
  setElemImpl<false, false>(base, key, val);
}

void TranslatorX64::emitSetElem(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                const MInstrInfo& mii, unsigned mInd,
                                unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  void (*setElemOp)(TypedValue*, TypedValue*, Cell*) =
    (ni.immVecM[mInd] == MEL)
    ? (setResult ? setElemLR : setElemL)
    : (setResult ? setElemCR : setElemC);
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  bool useRVal = (!forceMValIncDec(t, ni, mii) && val.isVariant());
  PREP_VAL(useRVal, rdx);
  // Emit the appropriate helper call.
  EMIT_RCALL(a, ni, setElemOp,
                    R(rBase),
                    ML(key.location, a, m_regMap, rsp),
                    IE(forceMValIncDec(t, ni, mii),
                       RPLUS(rsp, offsetof(MInstrState, tvVal)),
                       VAL(useRVal)));
}

template <bool unboxKey, bool setResult>
static inline void setPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                               Cell* val) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  SetProp<setResult>(ctx, base, key, val);
}

HOT_FUNC_VM
static void setPropLR(Class* ctx, TypedValue* base, TypedValue* key,
                      Cell* val) {
  setPropImpl<true, true>(ctx, base, key, val);
}

HOT_FUNC_VM
static void setPropL(Class* ctx, TypedValue* base, TypedValue* key, Cell* val) {
  setPropImpl<true, false>(ctx, base, key, val);
}

HOT_FUNC_VM
static void setPropCR(Class* ctx, TypedValue* base, TypedValue* key,
                      Cell* val) {
  setPropImpl<false, true>(ctx, base, key, val);
}

HOT_FUNC_VM
static void setPropC(Class* ctx, TypedValue* base, TypedValue* key, Cell* val) {
  setPropImpl<false, false>(ctx, base, key, val);
}

void TranslatorX64::emitSetProp(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                const MInstrInfo& mii, bool ctxFixed,
                                unsigned mInd, unsigned iInd,
                                const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  void (*setPropOp)(Class*, TypedValue*, TypedValue*, Cell*) =
    (ni.immVecM[mInd] == MPL)
    ? (setResult ? setPropLR : setPropL)
    : (setResult ? setPropCR : setPropC);
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  PREP_CTX(ctxFixed, rdi);
  bool useRVal = val.isVariant();
  PREP_VAL(useRVal, rcx);
  // Emit the appropriate helper call.
  EMIT_RCALL(a, ni, setPropOp,
                    CTX(ctxFixed),
                    R(rBase),
                    ML(key.location, a, m_regMap, rsp),
                    VAL(useRVal));
}

template <bool unboxKey, unsigned char op, bool setResult>
static inline void setOpElemImpl(TypedValue* base, TypedValue* key, Cell* val,
                                 TranslatorX64::MInstrState* mis,
                                 TypedValue* tvRes=NULL) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  TypedValue* result = SetOpElem(mis->tvScratch, mis->tvRef, op, base, key,
                                 val);
  if (setResult) {
    if (result->m_type == KindOfRef) {
      tvUnbox(result);
    }
    tvDup(result, tvRes); // tvRes may or may not be &mis->tvResult.
  }
}

#define SETOP_OP(op, bcOp) \
HOT_FUNC_VM \
static void setOp##op##ElemLR(TypedValue* base, TypedValue* key, Cell* val, \
                              TranslatorX64::MInstrState* mis, \
                              TypedValue* tvRes) { \
  setOpElemImpl<true, SetOp##op, true>(base, key, val, mis, tvRes); \
} \
HOT_FUNC_VM \
static void setOp##op##ElemCR(TypedValue* base, TypedValue* key, Cell* val, \
                              TranslatorX64::MInstrState* mis, \
                              TypedValue* tvRes) { \
  setOpElemImpl<false, SetOp##op, true>(base, key, val, mis, tvRes); \
} \
HOT_FUNC_VM \
static void setOp##op##ElemL(TypedValue* base, TypedValue* key, Cell* val, \
                             TranslatorX64::MInstrState* mis) { \
  setOpElemImpl<true, SetOp##op, false>(base, key, val, mis); \
} \
HOT_FUNC_VM \
static void setOp##op##ElemC(TypedValue* base, TypedValue* key, Cell* val, \
                             TranslatorX64::MInstrState* mis) { \
  setOpElemImpl<false, SetOp##op, false>(base, key, val, mis); \
}
SETOP_OPS
#undef SETOP_OP

void TranslatorX64::emitSetOpElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  unsigned char op = ni.imm[0].u_OA;
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  bool useRVal = (!forceMValIncDec(t, ni, mii) && val.isVariant());
  PREP_VAL(useRVal, rdx);
  // Emit the appropriate helper call.
  if (setResult) {
    void (*setOpElemOp)(TypedValue*, TypedValue*, Cell*, MInstrState*,
                        TypedValue*);
    switch (op) {
#define SETOP_OP(op, bcOp) \
      case SetOp##op: \
        setOpElemOp = (ni.immVecM[mInd] == MEL) \
                      ? setOp##op##ElemLR : setOp##op##ElemCR; \
        break;
      SETOP_OPS
#undef SETOP_OP
      default: not_reached();
    }
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, setOpElemOp,
                      R(rBase),
                      ML(key.location, a, m_regMap, rsp),
                      IE(forceMValIncDec(t, ni, mii),
                         RPLUS(rsp, offsetof(MInstrState, tvVal)),
                         VAL(useRVal)),
                      R(rsp),
                      RESULT(useTvR));
  } else {
    void (*setOpElemOp)(TypedValue*, TypedValue*, Cell*, MInstrState*);
    switch (op) {
#define SETOP_OP(op, bcOp) \
      case SetOp##op: \
        setOpElemOp = (ni.immVecM[mInd] == MEL) \
                      ? setOp##op##ElemL : setOp##op##ElemC; \
        break;
      SETOP_OPS
#undef SETOP_OP
      default: not_reached();
    }
    EMIT_RCALL(a, ni, setOpElemOp,
                      R(rBase),
                      ML(key.location, a, m_regMap, rsp),
                      IE(forceMValIncDec(t, ni, mii),
                         RPLUS(rsp, offsetof(MInstrState, tvVal)),
                         VAL(useRVal)),
                      R(rsp));
  }
}

template <bool unboxKey, unsigned char op, bool setResult>
static inline void setOpPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                 Cell* val, TranslatorX64::MInstrState* mis,
                                 TypedValue* tvRes=NULL) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  TypedValue* result = SetOpProp(mis->tvScratch, mis->tvRef, ctx, op, base, key,
                                 val);
  if (setResult) {
    if (result->m_type == KindOfRef) {
      tvUnbox(result);
    }
    tvDup(result, tvRes); // tvRes may or may not be &mis->tvResult.
  }
}

#define SETOP_OP(op, bcOp) \
HOT_FUNC_VM \
static void setOp##op##PropLR(Class* ctx, TypedValue* base, TypedValue* key, \
                              Cell* val, TranslatorX64::MInstrState* mis, \
                              TypedValue* tvRes) { \
  setOpPropImpl<true, SetOp##op, true>(ctx, base, key, val, mis, tvRes); \
} \
HOT_FUNC_VM \
static void setOp##op##PropCR(Class* ctx, TypedValue* base, TypedValue* key, \
                              Cell* val, TranslatorX64::MInstrState* mis, \
                              TypedValue* tvRes) { \
  setOpPropImpl<false, SetOp##op, true>(ctx, base, key, val, mis, tvRes); \
} \
HOT_FUNC_VM \
static void setOp##op##PropL(Class* ctx, TypedValue* base, TypedValue* key, \
                              Cell* val, TranslatorX64::MInstrState* mis) { \
  setOpPropImpl<true, SetOp##op, false>(ctx, base, key, val, mis); \
} \
HOT_FUNC_VM \
static void setOp##op##PropC(Class* ctx, TypedValue* base, TypedValue* key, \
                              Cell* val, TranslatorX64::MInstrState* mis) { \
  setOpPropImpl<false, SetOp##op, false>(ctx, base, key, val, mis); \
}
SETOP_OPS
#undef SETOP_OP

void TranslatorX64::emitSetOpProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, bool ctxFixed,
                                  unsigned mInd, unsigned iInd,
                                  const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  unsigned char op = ni.imm[0].u_OA;
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  PREP_CTX(ctxFixed, rdi);
  bool useRVal = val.isVariant();
  PREP_VAL(useRVal, rcx);
  // Emit the appropriate helper call.
  if (setResult) {
    void (*setOpPropOp)(Class*, TypedValue*, TypedValue*, Cell*, MInstrState*,
                        TypedValue*);
    switch (op) {
#define SETOP_OP(op, bcOp) \
      case SetOp##op: \
        setOpPropOp = (ni.immVecM[mInd] == MEL) \
                      ? setOp##op##PropLR : setOp##op##PropCR; \
        break;
      SETOP_OPS
#undef SETOP_OP
      default: not_reached();
    }
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, setOpPropOp,
                      CTX(ctxFixed),
                      R(rBase),
                      ML(key.location, a, m_regMap, rsp),
                      VAL(useRVal),
                      R(rsp),
                      RESULT(useTvR));
  } else {
    void (*setOpPropOp)(Class*, TypedValue*, TypedValue*, Cell*, MInstrState*);
    switch (op) {
#define SETOP_OP(op, bcOp) \
      case SetOp##op: \
        setOpPropOp = (ni.immVecM[mInd] == MEL) \
                      ? setOp##op##PropL : setOp##op##PropC; \
        break;
      SETOP_OPS
#undef SETOP_OP
      default: not_reached();
    }
    EMIT_RCALL(a, ni, setOpPropOp,
                      CTX(ctxFixed),
                      R(rBase),
                      ML(key.location, a, m_regMap, rsp),
                      VAL(useRVal),
                      R(rsp));
  }
}

template <bool unboxKey, unsigned char op, bool setResult>
static inline void incDecElemImpl(TypedValue* base, TypedValue* key,
                                  TranslatorX64::MInstrState* mis,
                                  TypedValue* tvRes) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  IncDecElem<setResult>(mis->tvScratch, mis->tvRef, op, base, key, *tvRes);
}

#define INCDEC_OP(op) \
HOT_FUNC_VM \
static void incDec##op##ElemLR(TypedValue* base, TypedValue* key, \
                               TranslatorX64::MInstrState* mis, \
                               TypedValue* tvRes) { \
  incDecElemImpl<true, op, true>(base, key, mis, tvRes); \
} \
HOT_FUNC_VM \
static void incDec##op##ElemCR(TypedValue* base, TypedValue* key, \
                               TranslatorX64::MInstrState* mis, \
                               TypedValue* tvRes) { \
  incDecElemImpl<false, op, true>(base, key, mis, tvRes); \
} \
HOT_FUNC_VM \
static void incDec##op##ElemL(TypedValue* base, TypedValue* key, \
                              TranslatorX64::MInstrState* mis) { \
  TypedValue tvRes; /* Not used; no need to initialize. */ \
  incDecElemImpl<true, op, false>(base, key, mis, &tvRes); \
} \
HOT_FUNC_VM \
static void incDec##op##ElemC(TypedValue* base, TypedValue* key, \
                              TranslatorX64::MInstrState* mis) { \
  TypedValue tvRes; /* Not used; no need to initialize. */ \
  incDecElemImpl<false, op, false>(base, key, mis, &tvRes); \
}
INCDEC_OPS
#undef INCDEC_OP

void TranslatorX64::emitIncDecElem(const Tracelet& t,
                                   const NormalizedInstruction& ni,
                                   const MInstrInfo& mii, unsigned mInd,
                                   unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  unsigned char op = ni.imm[0].u_OA;
  const DynLocation& key = *ni.inputs[iInd];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.smashLoc(key.location);
  // Emit the appropriate helper call.
  if (setResult) {
    void (*incDecElemOp)(TypedValue*, TypedValue*, MInstrState*, TypedValue*);
    switch (op) {
#define INCDEC_OP(op) \
      case op: \
        incDecElemOp = (ni.immVecM[mInd] == MEL) \
                       ? incDec##op##ElemLR : incDec##op##ElemCR; \
        break;
      INCDEC_OPS
#undef INCDEC_OP
      default: not_reached();
    }
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, incDecElemOp,
                      R(rBase),
                      ML(key.location, a, m_regMap, rsp),
                      R(rsp),
                      RESULT(useTvR));
  } else {
    void (*incDecElemOp)(TypedValue*, TypedValue*, MInstrState*);
    switch (op) {
#define INCDEC_OP(op) \
      case op: \
        incDecElemOp = (ni.immVecM[mInd] == MEL) \
                       ? incDec##op##ElemL : incDec##op##ElemC; \
        break;
      INCDEC_OPS
#undef INCDEC_OP
      default: not_reached();
    }
    EMIT_RCALL(a, ni, incDecElemOp, R(rBase),
               ML(key.location, a, m_regMap, rsp), R(rsp));
  }
}

template <bool unboxKey, unsigned char op, bool setResult>
static inline void incDecPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                  TranslatorX64::MInstrState* mis,
                                  TypedValue* tvRes) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  IncDecProp<setResult>(mis->tvScratch, mis->tvRef, ctx, op, base, key, *tvRes);
}

#define INCDEC_OP(op) \
HOT_FUNC_VM \
static void incDec##op##PropLR(Class* ctx, TypedValue* base, TypedValue* key, \
                               TranslatorX64::MInstrState* mis, \
                               TypedValue* tvRes) { \
  incDecPropImpl<true, op, true>(ctx, base, key, mis, tvRes); \
} \
HOT_FUNC_VM \
static void incDec##op##PropCR(Class* ctx, TypedValue* base, TypedValue* key, \
                               TranslatorX64::MInstrState* mis, \
                               TypedValue* tvRes) { \
  incDecPropImpl<false, op, true>(ctx, base, key, mis, tvRes); \
} \
HOT_FUNC_VM \
static void incDec##op##PropL(Class* ctx, TypedValue* base, TypedValue* key, \
                              TranslatorX64::MInstrState* mis) { \
  TypedValue tvRes; /* Not used; no need to initialize. */ \
  incDecPropImpl<true, op, false>(ctx, base, key, mis, &tvRes); \
} \
HOT_FUNC_VM \
static void incDec##op##PropC(Class* ctx, TypedValue* base, TypedValue* key, \
                              TranslatorX64::MInstrState* mis) { \
  TypedValue tvRes; /* Not used; no need to initialize. */ \
  incDecPropImpl<false, op, false>(ctx, base, key, mis, &tvRes); \
}
INCDEC_OPS
#undef INCDEC_OP

void TranslatorX64::emitIncDecProp(const Tracelet& t,
                                   const NormalizedInstruction& ni,
                                   const MInstrInfo& mii, bool ctxFixed,
                                   unsigned mInd, unsigned iInd,
                                   const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  unsigned char op = ni.imm[0].u_OA;
  const DynLocation& key = *ni.inputs[iInd];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.smashLoc(key.location);
  PREP_CTX(ctxFixed, rdi);
  // Emit the appropriate helper call.
  if (setResult) {
    void (*incDecPropOp)(Class*, TypedValue*, TypedValue*, MInstrState*,
                         TypedValue*);
    switch (op) {
#define INCDEC_OP(op) \
      case op: \
        incDecPropOp = (ni.immVecM[mInd] == MEL) \
                       ? incDec##op##PropLR : incDec##op##PropCR; \
        break;
      INCDEC_OPS
#undef INCDEC_OP
      default: not_reached();
    }
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, incDecPropOp,
                      CTX(ctxFixed),
                      R(rBase),
                      ML(key.location, a, m_regMap, rsp),
                      R(rsp),
                      RESULT(useTvR));
  } else {
    void (*incDecPropOp)(Class*, TypedValue*, TypedValue*, MInstrState*);
    switch (op) {
#define INCDEC_OP(op) \
      case op: \
        incDecPropOp = (ni.immVecM[mInd] == MEL) \
                       ? incDec##op##PropL : incDec##op##PropC; \
        break;
      INCDEC_OPS
#undef INCDEC_OP
      default: not_reached();
    }
    EMIT_RCALL(a, ni, incDecPropOp,
                      CTX(ctxFixed),
                      R(rBase),
                      ML(key.location, a, m_regMap, rsp),
                      R(rsp));
  }
}

template <bool unboxKey>
static inline void bindElemImpl(TypedValue* base, TypedValue* key, RefData* val,
                                TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  base = ElemD<false, true>(mis->tvScratch, mis->tvRef, base, key);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val->tv(), base);
  }
}

HOT_FUNC_VM
static void bindElemL(TypedValue* base, TypedValue* key, RefData* val,
                      TranslatorX64::MInstrState* mis) {
  bindElemImpl<true>(base, key, val, mis);
}

HOT_FUNC_VM
static void bindElemC(TypedValue* base, TypedValue* key, RefData* val,
                      TranslatorX64::MInstrState* mis) {
  bindElemImpl<false>(base, key, val, mis);
}

void TranslatorX64::emitBindElem(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, unsigned mInd,
                                 unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  ASSERT(generateMVal(t, ni, mii));
  void (*bindElemOp)(TypedValue*, TypedValue*, RefData*, MInstrState*) =
    (ni.immVecM[mInd] == MEL) ? bindElemL : bindElemC;
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  ASSERT(!forceMValIncDec(t, ni, mii));
  ASSERT(val.isVariant());
  EMIT_RCALL(a, ni, bindElemOp, R(rBase), ML(key.location, a, m_regMap, rsp),
             A(val.location), R(rsp));
}

template <bool unboxKey>
static inline void bindPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                RefData* val, TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  base = Prop<false, true, false>(mis->tvScratch, mis->tvRef, ctx, base, key);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val->tv(), base);
  }
}

HOT_FUNC_VM
static inline void bindPropL(Class* ctx, TypedValue* base, TypedValue* key,
                             RefData* val, TranslatorX64::MInstrState* mis) {
  bindPropImpl<true>(ctx, base, key, val, mis);
}

HOT_FUNC_VM
static inline void bindPropC(Class* ctx, TypedValue* base, TypedValue* key,
                            RefData* val, TranslatorX64::MInstrState* mis) {
  bindPropImpl<false>(ctx, base, key, val, mis);
}

void TranslatorX64::emitBindProp(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, bool ctxFixed,
                                 unsigned mInd, unsigned iInd,
                                 const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  ASSERT(generateMVal(t, ni, mii));
  void (*bindPropOp)(Class*, TypedValue*, TypedValue*, RefData*, MInstrState*) =
    (ni.immVecM[mInd] == MPL) ? bindPropL : bindPropC;
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  PREP_CTX(ctxFixed, rdi);
  // Emit the appropriate helper call.
  ASSERT(!forceMValIncDec(t, ni, mii));
  ASSERT(val.isVariant());
  EMIT_RCALL(a, ni, bindPropOp,
                    CTX(ctxFixed),
                    R(rBase),
                    ML(key.location, a, m_regMap, rsp),
                    A(val.location),
                    R(rsp));
}

template <bool unboxKey>
static inline void unsetElemImpl(TypedValue* base, TypedValue* key) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  UnsetElem(base, key);
}

HOT_FUNC_VM
static void unsetElemL(TypedValue* base, TypedValue* key) {
  unsetElemImpl<true>(base, key);
}

HOT_FUNC_VM
static void unsetElemC(TypedValue* base, TypedValue* key) {
  unsetElemImpl<false>(base, key);
}

void TranslatorX64::emitUnsetElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  void (*unsetElemOp)(TypedValue*, TypedValue*) =
    ni.immVecM[mInd] == MEL ? unsetElemL : unsetElemC;
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  EMIT_RCALL(a, ni, unsetElemOp, R(rBase), ML(key.location, a, m_regMap, rsp));
}

template <bool unboxKey>
static inline void unsetPropImpl(Class* ctx, TypedValue* base,
                                 TypedValue* key) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  UnsetProp(ctx, base, key);
}

HOT_FUNC_VM
static void unsetPropL(Class* ctx, TypedValue* base, TypedValue* key) {
  unsetPropImpl<true>(ctx, base, key);
}

HOT_FUNC_VM
static void unsetPropC(Class* ctx, TypedValue* base, TypedValue* key) {
  unsetPropImpl<false>(ctx, base, key);
}

void TranslatorX64::emitUnsetProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, bool ctxFixed,
                                  unsigned mInd, unsigned iInd,
                                  const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  void (*unsetPropOp)(Class*, TypedValue*, TypedValue*) =
    ni.immVecM[mInd] == MPL ? unsetPropL : unsetPropC;
  m_regMap.smashLoc(key.location);
  m_regMap.smashLoc(val.location);
  PREP_CTX(ctxFixed, rdi);
  // Emit the appropriate helper call.
  EMIT_RCALL(a, ni, unsetPropOp,
                    CTX(ctxFixed),
                    R(rBase),
                    ML(key.location, a, m_regMap, rsp));
}

static inline void vGetNewElem(TypedValue* base, TypedValue* result,
                               TranslatorX64::MInstrState* mis) {
  base = NewElem(mis->tvScratch, mis->tvRef, base);
  if (base == &mis->tvScratch && base->m_type == KindOfUninit) {
    // Error (no result was set).
    tvWriteNull(result);
    tvBox(result);
  } else {
    if (base->m_type != KindOfRef) {
      tvBox(base);
    }
    tvDupVar(base, result);
  }
}

void TranslatorX64::emitVGetNewElem(const Tracelet& t,
                                    const NormalizedInstruction& ni,
                                    const MInstrInfo& mii, unsigned mInd,
                                    unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  void (*vGetNewElemOp)(TypedValue*, TypedValue*, MInstrState*) = vGetNewElem;
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  EMIT_RCALL(a, ni, vGetNewElemOp,
                    R(rBase),
                    RESULT(useTvR),
                    R(rsp));
}

HOT_FUNC_VM
static void setNewElemR(TypedValue* base, Cell* val) {
  SetNewElem<true>(base, val);
}

HOT_FUNC_VM
static void setNewElem(TypedValue* base, Cell* val) {
  SetNewElem<false>(base, val);
}

void TranslatorX64::emitSetNewElem(const Tracelet& t,
                                   const NormalizedInstruction& ni,
                                   const MInstrInfo& mii, unsigned mInd,
                                   unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  const DynLocation& val = *ni.inputs[0];
  m_regMap.smashLoc(val.location);
  // Emit the appropriate helper call.
  void (*setNewElemOp)(TypedValue*, Cell*) =
    setResult ? setNewElemR : setNewElem;
  bool useRVal = val.isVariant();
  PREP_VAL(useRVal, rsi);
  EMIT_RCALL(a, ni, setNewElemOp,
                    R(rBase),
                    VAL(useRVal));
}

template <unsigned char op, bool setResult>
static inline void setOpNewElemImpl(TypedValue* base, Cell* val,
                                    TranslatorX64::MInstrState* mis,
                                    TypedValue* tvRes=NULL) {
  TypedValue* result = SetOpNewElem(mis->tvScratch, mis->tvRef, op, base, val);
  if (setResult) {
    if (result->m_type == KindOfRef) {
      tvUnbox(result);
    }
    tvDup(result, tvRes); // tvRes may or may not be &mis->tvResult.
  }
}

#define SETOP_OP(op, bcOp) \
HOT_FUNC_VM \
static void setOp##op##NewElemR(TypedValue* base, Cell* val, \
                                TranslatorX64::MInstrState* mis, \
                                TypedValue* tvRes) { \
  setOpNewElemImpl<SetOp##op, true>(base, val, mis, tvRes); \
} \
HOT_FUNC_VM \
static void setOp##op##NewElem(TypedValue* base, Cell* val, \
                               TranslatorX64::MInstrState* mis) { \
  setOpNewElemImpl<SetOp##op, false>(base, val, mis); \
}
SETOP_OPS
#undef SETOP_OP

void TranslatorX64::emitSetOpNewElem(const Tracelet& t,
                                     const NormalizedInstruction& ni,
                                     const MInstrInfo& mii, unsigned mInd,
                                     unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  unsigned char op = ni.imm[0].u_OA;
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.smashLoc(val.location);
  bool useRVal = val.isVariant();
  PREP_VAL(useRVal, rcx);
  // Emit the appropriate helper call.
  if (setResult) {
    void (*setOpNewElemOp)(TypedValue*, Cell*, MInstrState*, TypedValue*);
    switch (op) {
#define SETOP_OP(op, bcOp) \
    case SetOp##op: setOpNewElemOp = setOp##op##NewElemR; break;
    SETOP_OPS
#undef SETOP_OP
    default: not_reached();
    }
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, setOpNewElemOp,
                      R(rBase),
                      VAL(useRVal),
                      R(rsp),
                      RESULT(useTvR));
  } else {
    void (*setOpNewElemOp)(TypedValue*, Cell*, MInstrState*);
    switch (op) {
#define SETOP_OP(op, bcOp) \
    case SetOp##op: setOpNewElemOp = setOp##op##NewElem; break;
    SETOP_OPS
#undef SETOP_OP
    default: not_reached();
    }
    EMIT_RCALL(a, ni, setOpNewElemOp,
                      R(rBase),
                      VAL(useRVal),
                      R(rsp));
  }
}

template <unsigned char op, bool setResult>
static inline void incDecNewElemImpl(TypedValue* base,
                                     TranslatorX64::MInstrState* mis,
                                     TypedValue* tvRes) {
  IncDecNewElem<setResult>(mis->tvScratch, mis->tvRef, op, base, *tvRes);
}

#define INCDEC_OP(op) \
HOT_FUNC_VM \
static void incDec##op##NewElemR(TypedValue* base, \
                                 TranslatorX64::MInstrState* mis, \
                                 TypedValue* tvRes) { \
  incDecNewElemImpl<op, true>(base, mis, tvRes); \
} \
HOT_FUNC_VM \
static void incDec##op##NewElem(TypedValue* base, \
                                TranslatorX64::MInstrState* mis) { \
  TypedValue tvRes; /* Not used; no need to initialize. */ \
  incDecNewElemImpl<op, false>(base, mis, &tvRes); \
}
INCDEC_OPS
#undef INCDEC_OP

void TranslatorX64::emitIncDecNewElem(const Tracelet& t,
                                      const NormalizedInstruction& ni,
                                      const MInstrInfo& mii, unsigned mInd,
                                      unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  unsigned char op = ni.imm[0].u_OA;
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  // Emit the appropriate helper call.
  if (setResult) {
    void (*incDecNewElemOp)(TypedValue*, MInstrState*, TypedValue*);
    switch (op) {
#define INCDEC_OP(op) \
    case op: incDecNewElemOp = incDec##op##NewElemR; break;
    INCDEC_OPS
#undef INCDEC_OP
    default: not_reached();
    }
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, incDecNewElemOp,
                      R(rBase),
                      R(rsp),
                      RESULT(useTvR));
  } else {
    void (*incDecNewElemOp)(TypedValue*, MInstrState*);
    switch (op) {
#define INCDEC_OP(op) \
    case op: incDecNewElemOp = incDec##op##NewElem; break;
    INCDEC_OPS
#undef INCDEC_OP
    default: not_reached();
    }
    EMIT_RCALL(a, ni, incDecNewElemOp, R(rBase), R(rsp));
  }
}

static inline void bindNewElem(TypedValue* base, RefData* val,
                               TranslatorX64::MInstrState* mis) {
  base = NewElem(mis->tvScratch, mis->tvRef, base);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val->tv(), base);
  }
}

void TranslatorX64::emitBindNewElem(const Tracelet& t,
                                    const NormalizedInstruction& ni,
                                    const MInstrInfo& mii, unsigned mInd,
                                    unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  ASSERT(generateMVal(t, ni, mii));
  const DynLocation& val = *ni.inputs[0];
  m_regMap.smashLoc(val.location);
  // Emit the appropriate helper call.
  void (*bindNewElemOp)(TypedValue*, RefData*, MInstrState*) = bindNewElem;
  ASSERT(val.isVariant());
  EMIT_RCALL(a, ni, bindNewElemOp, R(rBase), A(val.location), R(rsp));
}

void TranslatorX64::emitNotSuppNewElem(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii, unsigned mInd,
                                       unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  not_reached();
}

#define MII(instr, attrs, bS, iS, vC, fN) \
void TranslatorX64::emitFinal##instr##MOp(const Tracelet& t, \
                                          const NormalizedInstruction& ni, \
                                          const MInstrInfo& mii, \
                                          bool ctxFixed, unsigned mInd, \
                                          unsigned iInd, \
                                          const PhysReg& rBase) { \
  switch (ni.immVecM[mInd]) { \
  case MEC: case MEL: case MET: case MEI: \
    emit##instr##Elem(t, ni, mii, mInd, iInd, rBase); \
    break; \
  case MPC: case MPL: case MPT: \
    emit##instr##Prop(t, ni, mii, ctxFixed, mInd, iInd, rBase); \
    break; \
  case MW: \
    ASSERT((attrs) & MIA_final); \
    emit##fN(t, ni, mii, mInd, iInd, rBase); \
    break; \
  default: not_reached(); \
  } \
}
MINSTRS
#undef MII

static void getMInstrCtx(TranslatorX64::MInstrState* mis) {
  VMRegAnchor _;
  mis->ctx = arGetContextClass(curFrame());
}

bool TranslatorX64::needMInstrCtx(const Tracelet& t,
                                  const NormalizedInstruction& ni) const {
  if (false && !isContextFixed()) {
    SKTRACE(2, ni.source, "%s (context not fixed) --> true\n", __func__);
    return true;
  }
  // Return true if the context will actually be used; otherwise return false
  // in order to avoid emission of getMInstrCtx() calls.
  switch (ni.immVec.locationCode()) {
  case LL: case LC: case LR:
  case LGL: case LGC:
  case LNL: case LNC: break;
  case LSL: case LSC: {
    SKTRACE(2, ni.source, "%s (location uses context) --> true\n", __func__);
    return true;
  }
  default:            not_reached();
  }
  for (unsigned m = 0; m < ni.immVecM.size(); ++m) {
    switch (ni.immVecM[m]) {
    case MEC: case MEL:
    case MET: case MEI:
    case MW:                      break;
    case MPC: case MPL: case MPT: {
      SKTRACE(2, ni.source, "%s (member %u uses context) --> true\n",
              __func__, m);
      return true;
    }
    default:                      not_reached();
    }
  }
  SKTRACE(2, ni.source, "%s --> false\n", __func__);
  return false;
}

void TranslatorX64::emitMPre(const Tracelet& t,
                             const NormalizedInstruction& ni,
                             const MInstrInfo& mii, bool& ctxFixed,
                             unsigned& mInd, unsigned& iInd, PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx\n", __func__, long(a.code.frontier));
  a.    sub_imm32_reg64(sizeof(MInstrState), rsp);
  emitStoreUninitNull(a, offsetof(MInstrState, tvScratch), rsp);
  if (debug) {
    emitStoreInvalid(a, offsetof(MInstrState, tvLiteral), rsp);
  }
  SKTRACE(2, ni.source, "%s nLogicalRatchets=%u\n",
          __func__, nLogicalRatchets(t, ni, mii));
  if (nLogicalRatchets(t, ni, mii) > 0) {
    emitStoreUninitNull(a, offsetof(MInstrState, tvRef), rsp);
    emitStoreUninitNull(a, offsetof(MInstrState, tvRef2), rsp);
  } else if (debug) {
    emitStoreInvalid(a, offsetof(MInstrState, tvRef), rsp);
    emitStoreInvalid(a, offsetof(MInstrState, tvRef2), rsp);
  }
  if (useTvResult(t, ni, mii)) {
    emitStoreUninitNull(a, offsetof(MInstrState, tvResult), rsp);
  } else if (debug) {
    emitStoreInvalid(a, offsetof(MInstrState, tvResult), rsp);
  }
  a.    store_imm32_disp_reg(false, offsetof(MInstrState, baseStrOff), rsp);

  ctxFixed = isContextFixed();
  SKTRACE(2, ni.source, "%s ctxFixed=%s\n",
          __func__, ctxFixed ? "true" : "false");

  /* XXX Im pretty sure we shouldnt need to clean/smash the regs */
  m_regMap.cleanRegs(kAllRegs);
  m_regMap.smashRegs(kCallerSaved);

  if (!ctxFixed && needMInstrCtx(t, ni)) {
    EMIT_CALL(a, getMInstrCtx, R(rsp));
    recordCall(ni);
  } else if (debug) {
    a.  store_imm64_disp_reg64(0xfacefacefacefaceULL,
                               offsetof(MInstrState, ctx), rsp);
  }

  // Check if val incref/decref is forced as a side effect of analysis
  // converting the val from a stack input to a local input.
  if (forceMValIncDec(t, ni, mii)) {
    SKTRACE(2, ni.source, "%s %#lx force val incref\n",
            __func__, long(a.code.frontier));
    const DynLocation& val = *ni.inputs[0];
    LazyScratchReg tmp(m_regMap);
    PhysReg rVal = m_regMap.hasReg(val.location)
                   ? m_regMap.getReg(val.location)
                   : m_regMap.allocReg(val.location, val.outerType(),
                                       RegInfo::CLEAN);
    if (val.isVariant()) {
      tmp.alloc();
      emitDeref(a, rVal, *tmp);
      rVal = *tmp;
    }
    // Copy val to tvVal for later assignment and decref.
    ASSERT(val.valueType() == KindOfArray);
    emitStoreTypedValue(a, KindOfArray, rVal, offsetof(MInstrState, tvVal),
                        rsp);
    // Incref, offset by decref in emitMPost().
    emitIncRef(rVal, KindOfArray);
  }

  // The base location is input 0 or 1, and the location code is stored
  // separately from ni.immVecM, so input indices (iInd) and member indices
  // (mInd) commonly differ.  Additionally, W members have no corresponding
  // inputs, so it is necessary to track the two indices separately.
  iInd = mii.valCount();
  emitBaseOp(t, ni, mii, iInd, ctxFixed, rBase);
  ++iInd;
  // Iterate over all but the last member, which is consumed by a final
  // operation.
  for (mInd = 0; mInd < ni.immVecM.size() - 1; ++mInd) {
    emitIntermediateOp(t, ni, mii, ctxFixed, mInd, iInd, rBase);
    emitRatchetRefs(t, ni, mii, mInd, rBase);
  }
}

void TranslatorX64::emitMPost(const Tracelet& t,
                              const NormalizedInstruction& ni,
                              const MInstrInfo& mii) {
  SKTRACE(2, ni.source, "%s %#lx\n", __func__, long(a.code.frontier));
  // Decref stack inputs.  Some instructions push their topmost input as their
  // final result; skip input 0 if it is a result.
  for (unsigned i = logicalTeleportMVal(t, ni, mii) ? 1 : 0;
       i < ni.inputs.size();
       ++i) {
    const DynLocation& input = *ni.inputs[i];
    switch (input.location.space) {
    case Location::Stack: {
      DataType dt = input.outerType();
      if (IS_REFCOUNTED_TYPE(dt)) {
        SKTRACE(2, ni.source, "%s %#lx decref stack input %u, type %s\n",
                __func__, long(a.code.frontier), i, tname(dt).c_str());
        PhysReg pr = m_regMap.allocReg(input.location, dt, RegInfo::CLEAN);
        emitDecRef(ni, pr, dt);
        m_regMap.smashLoc(input.location);
      }
      break;
    }
    case Location::Local:
    case Location::Litstr:
    case Location::Litint: {
      // Do nothing.
      SKTRACE(2, ni.source, "%s (no decref needed) input %u, loc(%s, %lld)\n",
              __func__, i, input.location.spaceName(), input.location.offset);
      break;
    }
    default: not_reached();
    }
  }
  // Decref the val if incref/decref is forced as a side effect of analysis
  // converting the val from a stack input to a local input.
  if (forceMValIncDec(t, ni, mii)) {
    SKTRACE(2, ni.source, "%s %#lx force val decref\n",
            __func__, long(a.code.frontier));
    ScratchReg rScratch(m_regMap);
    a.  load_reg64_disp_reg64(rsp, offsetof(MInstrState, tvVal) + TVOFF(m_data),
                              *rScratch);
    emitDecRef(ni, *rScratch, KindOfArray);
  }
  // Clean up ratchet.
  if (nLogicalRatchets(t, ni, mii) > 1) {
    SKTRACE(2, ni.source, "%s %#lx decref tvRef2\n",
            __func__, long(a.code.frontier));
    emitDecRefGeneric(ni, rsp, offsetof(MInstrState, tvRef2));
  }
  if (nLogicalRatchets(t, ni, mii) > 0) {
    SKTRACE(2, ni.source, "%s %#lx decref tvRef\n",
            __func__, long(a.code.frontier));
    emitDecRefGeneric(ni, rsp, offsetof(MInstrState, tvRef));
  }
  // Copy tvResult to final location if it was used.
  if (useTvResult(t, ni, mii)) {
    SKTRACE(2, ni.source, "%s %#lx copy tvResult\n",
            __func__, long(a.code.frontier));
    ScratchReg rTvResult(m_regMap);
    a.  lea_reg64_disp_reg64(rsp, offsetof(MInstrState, tvResult), *rTvResult);
    ScratchReg rScratch(m_regMap);
    emitCopyToStackRegSafe(a, ni, *rTvResult, mResultStackOffset(ni),
                           *rScratch);
  }
  // Teleport val to final location if this instruction generates val.
  if (teleportMVal(t, ni, mii)) {
    ASSERT(!useTvResult(t, ni, mii));
    SKTRACE(2, ni.source, "%s %#lx teleport val\n",
            __func__, long(a.code.frontier));
    ScratchReg rScratch(m_regMap);
    const DynLocation& val = *ni.inputs[0];
    m_regMap.smashLoc(val.location);
    PhysReg prVal;
    int dispVal;
    locToRegDisp(val.location, &prVal, &dispVal);
    emitCopyTo(a, prVal, dispVal, rVmSp,
               vstackOffset(ni, mResultStackOffset(ni)), *rScratch);
  }
  a.    add_imm32_reg64(sizeof(MInstrState), rsp);
}

template <bool unboxKey>
static inline void cGetElemImpl(TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                TranslatorX64::MInstrState* mis) {
  if (unboxKey) {
    if (key->m_type == KindOfRef) {
      key = key->m_data.pref->tv();
    }
  }
  base = Elem<true>(*result, mis->tvRef, base, mis->baseStrOff, key);
  if (base != result) {
    // Save a copy of the result.
    tvDup(base, result);
  }
  if (result->m_type == KindOfRef) {
    tvUnbox(result);
  }
}

HOT_FUNC_VM
static void cGetElemL(TypedValue* base, TypedValue* key, TypedValue* result,
                      TranslatorX64::MInstrState* mis) {
  cGetElemImpl<true>(base, key, result, mis);
}

HOT_FUNC_VM
static void cGetElemC(TypedValue* base, TypedValue* key, TypedValue* result,
                      TranslatorX64::MInstrState* mis) {
  cGetElemImpl<false>(base, key, result, mis);
}

void TranslatorX64::emitCGetElem(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, unsigned mInd,
                                 unsigned iInd, const PhysReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& memb = *ni.inputs[iInd];
  bool isVariant = memb.isVariant();
  // Slight optimization: we can use cGetElemC (which doesn't check for
  // key unboxing) if we know our input isn't variant.
  auto cGetElemOp = (isVariant && ni.immVecM[mInd]) == MEL ? cGetElemL
    : cGetElemC;
  ASSERT(IMPLIES(isVariant, ni.immVecM[mInd] == MEL));
  
  m_regMap.smashLoc(memb.location);
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  EMIT_RCALL(a, ni, cGetElemOp,
                    R(rBase),
                    ML(memb.location, a, m_regMap, rsp),
                    RESULT(useTvR),
                    R(rsp));
}

bool
isNormalPropertyAccess(const NormalizedInstruction& i,
                       int propInput,
                       int objInput) {
  const LocationCode lcode = i.immVec.locationCode();
  return
    i.immVecM.size() == 1 &&
    (lcode == LC || lcode == LL || lcode == LR) &&
    mcodeMaybePropName(i.immVecM[0]) &&
    i.inputs[propInput]->isString() &&
    i.inputs[objInput]->valueType() == KindOfObject;
}

Slot
getPropertyOffset(const NormalizedInstruction& i,
                  int propInput, int objInput) {
  ASSERT(isNormalPropertyAccess(i, propInput, objInput));
  // The context class must be fixed
  if (!isContextFixed()) {
    return kInvalidSlot;
  }
  // Property name and base class must be known at compile time
  const StringData* name = i.inputs[propInput]->rtt.valueString();
  const Class* baseClass = i.inputs[objInput]->rtt.valueClass();
  if (name == NULL || baseClass == NULL) {
    return kInvalidSlot;
  }
  bool accessible;
  Class* ctx = curFunc()->cls();
  // If we are not in repo-authoriative mode, we need to check that
  // baseClass cannot change in between requests
  if (!RuntimeOption::RepoAuthoritative ||
      !(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return kInvalidSlot;
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return kInvalidSlot;
      }
    }
  }
  // Lookup the index of the property based on ctx and baseClass
  Slot idx = baseClass->getDeclPropIndex(ctx, name, accessible);
  // If we couldn't find a property that is accessible in the current
  // context, bail out
  if (idx == kInvalidSlot || !accessible) {
    return kInvalidSlot;
  }
  // If it's a declared property we're good to go: even if a subclass
  // redefines an accessible property with the same name it's guaranteed
  // to be at the same offset
  return baseClass->declPropOffset(idx);
}

bool
isSupportedCGetMProp(const NormalizedInstruction& i) {
  if (i.inputs.size() != 2) return false;
  SKTRACE(2, i.source, "CGetM prop candidate: prop supported: %d, "
                       "in[0] %s in[1] %s\n",
          mcodeMaybePropName(i.immVecM[0]),
          i.inputs[0]->rtt.pretty().c_str(),
          i.inputs[1]->rtt.pretty().c_str());
  return isNormalPropertyAccess(i, 1, 0) && !curFunc()->isPseudoMain();
}

bool
isSupportedCGetM_LE(const NormalizedInstruction& i) {
  if (i.inputs.size() > 2) return false;

  // Return true iff this is a CGetM <H E> instruction where
  // the base is an array and the key is an int or string
  return
    i.immVec.locationCode() == LL &&
    i.immVecM.size() == 1 &&
    mcodeMaybeArrayKey(i.immVecM[0]) &&
    i.inputs[0]->valueType() == KindOfArray && // base
    (i.inputs[1]->isInt() || i.inputs[1]->isString()); // key
}

bool
isSupportedCGetM_LEE(const NormalizedInstruction& i) {
  if (i.inputs.size() > 3) return false;

  // Return true iff this is a CGetM <L E E> instruction where the
  // base is an array, the first key is a int and the second key is a
  // string.
  return
    i.immVec.locationCode() == LL &&
    i.immVecM.size() == 2 &&
    mcodeMaybeArrayIntKey(i.immVecM[0]) &&
    mcodeMaybeArrayStringKey(i.immVecM[1]) &&
    i.inputs[0]->valueType() == KindOfArray &&
    i.inputs[1]->isInt() && // first key
    i.inputs[2]->isString(); // second key
}

bool
isSupportedCGetM_RE(const NormalizedInstruction& i) {
  if (i.inputs.size() > 2) return false;
  return
    i.immVec.locationCode() == LR &&
    i.immVecM.size() == 1 &&
    mcodeMaybeArrayKey(i.immVecM[0]) &&
    i.inputs[0]->valueType() == KindOfArray && // base
    (i.inputs[1]->isInt() || i.inputs[1]->isString()); // key;
}

bool
isSupportedCGetM_GE(const NormalizedInstruction& i) {
  if (i.inputs.size() > 2) return false;
  const LocationCode lcode = i.immVec.locationCode();
  return
    (lcode == LGC || lcode == LGL) &&
    i.immVecM.size() == 1 &&
    mcodeMaybeArrayKey(i.immVecM[0]) &&
    i.inputs[0]->isString() && // name input
    (i.inputs[1]->isInt() || i.inputs[1]->isString()); // key input
}

void
TranslatorX64::analyzeCGetM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = supportedPlan(isSupportedCGetMProp(ni) ||
                                 isSupportedCGetM_LEE(ni) ||
                                 isSupportedCGetM_GE(ni) ||
                                 isSupportedCGetM_LE(ni) ||
                                 isSupportedCGetM_RE(ni));
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

static String propCacheName(const StringData* name) {
  if (name == NULL) {
    return String();
  }
  return String(string(name->data()) + ":" + Util::toLower(getContextName()));
}

void
TranslatorX64::emitPropGet(const NormalizedInstruction& i,
                           const DynLocation& base,
                           PhysReg fieldAddr,
                           const Location& outLoc) {
  ASSERT(outLoc.isStack());
  PhysReg baseReg = getReg(base.location);

  emitDerefIfVariant(a, fieldAddr);
  // We may be creating a reference to the field.
  emitIncRefGeneric(fieldAddr, 0);

  PhysReg stackOutReg;
  ScratchReg scratch(m_regMap);
  int stackOutDisp;
  locToRegDisp(outLoc, &stackOutReg, &stackOutDisp);
  emitCopyTo(a, fieldAddr, 0, stackOutReg, stackOutDisp, *scratch);

  if (!base.isLocal()) {
    // Release the base
    emitDecRef(i, baseReg, base.outerType());
  }
}

static void
raiseUndefProp(ObjectData* base, const StringData* name) {
  VMRegAnchor _;
  static_cast<Instance*>(base)->raiseUndefProp(name);
}

void
TranslatorX64::translateCGetMProp(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  using namespace TargetCache;
  ASSERT(i.inputs.size() == 2 && i.outStack);

  const int kBaseIdx = 0;
  const DynLocation& base = *i.inputs[kBaseIdx];
  const DynLocation& prop = *i.inputs[1];
  const Location& baseLoc = base.location;
  const Location& propLoc = prop.location;
  const Location& outLoc  = i.outStack->location;
  const Slot propOffset   = getPropertyOffset(i, 1, 0);

  if (propOffset != kInvalidSlot && i.immVec.locationCode() == LC) {
    m_regMap.allocInputReg(i, 0);
    ASSERT(!base.isLocal() && !base.isVariant());
    Stats::emitInc(a, Stats::Tx64_PropGetFast);
    ScratchReg fieldAddr(m_regMap);
    a.lea_reg64_disp_reg64(getReg(baseLoc), int(propOffset), *fieldAddr);
    // Still have to check for uninit
    a.cmp_imm32_disp_reg32(KindOfUninit, TVOFF(m_type), *fieldAddr);
    {
      UnlikelyIfBlock<CC_Z> ifUninit(a, astubs);
      EMIT_CALL(astubs, raiseUndefProp,
                V(baseLoc), IMM((uint64_t)prop.rtt.valueString()));
      recordReentrantStubCall(i);
      emitImmReg(astubs, (intptr_t)&init_null_variant, *fieldAddr);
    }
    emitPropGet(i, base, *fieldAddr, outLoc);
  } else {
    int args[2];
    args[0] = base.isVariant() ? ArgAnyReg : 1;
    args[1] = 2;
    allocInputsForCall(i, args);
    Stats::emitInc(a, Stats::Tx64_PropGetSlow);
    bool useCtx = !isContextFixed();
    const StringData* name = prop.rtt.valueString();
    ASSERT(name == NULL || name->isStatic());

    CacheHandle ch;
    TargetCache::pcb_lookup_func_t lookupFn = propLookupPrep(
      ch, useCtx ? name : propCacheName(name).get(),
      base.isLocal() ? BASE_LOCAL : BASE_CELL,
      useCtx ? DYN_CONTEXT : STATIC_CONTEXT,
      name ? STATIC_NAME : DYN_NAME);
    if (useCtx) {
      EMIT_CALL(a,
                 lookupFn,
                 IMM(ch),
                 base.isVariant() ? DEREF(baseLoc) : V(baseLoc),
                 V(propLoc),
                 A(outLoc),
                 R(rVmFp));
    } else {
      EMIT_CALL(a,
                 lookupFn,
                 IMM(ch),
                 base.isVariant() ? DEREF(baseLoc) : V(baseLoc),
                 V(propLoc),
                 A(outLoc));
    }
    recordReentrantCall(i);
  }
  m_regMap.invalidate(i.outStack->location);
}

void
TranslatorX64::emitArrayElem(const NormalizedInstruction& i,
                             const DynLocation* baseInput,
                             PhysReg baseReg,
                             const DynLocation* keyIn,
                             const Location& outLoc) {
  // Let the array helpers handle refcounting logic: key down,
  // return value up.
  SKTRACE(1, i.source, "emitCGetM: committed to unary load\n");
  bool decRefBase = baseInput->isStack();
  PhysReg decRefReg = noreg;
  if (decRefBase) {
    // We'll need to decref the base after the call. Make sure we hold
    // on to the value if it's a variant or from a global.
    if (isSupportedCGetM_GE(i)) {
      decRefReg = baseReg;
    } else if (baseInput->isVariant()) {
      decRefReg = getReg(baseInput->location);
    }
    if (decRefReg != noreg && kCallerSaved.contains(decRefReg)) {
      a.    pushr(decRefReg);
      a.    sub_imm32_reg64(8, rsp);
    }
  }

  void* fptr;
  if (keyIn->isInt()) {
    fptr = (void*)array_getm_i;
  } else {
    ASSERT(keyIn->isString());
    bool decRefKey = keyIn->isStack();
    bool doIntCheck = !i.hasConstImm;
    fptr = (decRefKey ?
            (doIntCheck ? (void*)array_getm_s : (void*)array_getm_s_fast) :
            (doIntCheck ? (void*)array_getm_s0 : (void*)array_getm_s0_fast));
  }
  if (false) { // type-check
    void *a = NULL;
    TypedValue tv;
    array_getm_i(a, 1, &tv);
    StringData *sd = NULL;
    array_getm_s(a, sd, &tv);
    array_getm_s0(a, sd, &tv);
  }
  EMIT_CALL(a, fptr, R(baseReg), V(keyIn->location), A(outLoc));
  recordReentrantCall(i);
  m_regMap.invalidate(outLoc);

  if (decRefBase) {
    // For convenience of decRefs, the callees return the ArrayData*...
    PhysReg base = rax;
    // but if it was boxed or from a global, we need to get the
    // original address back...
    if (decRefReg != noreg) {
      if (kCallerSaved.contains(decRefReg)) {
        a.    add_imm32_reg64(8, rsp);
        a.    popr(rax);
      } else {
        base = decRefReg;
      }
    }
    emitDecRef(i, base, KindOfArray);
  }
}

void
TranslatorX64::translateCGetM_LEE(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  ASSERT(isSupportedCGetM_LEE(i));

  const DynLocation& array = *i.inputs[0];
  const DynLocation& key1  = *i.inputs[1];
  const DynLocation& key2  = *i.inputs[2];
  const Location& outLoc    = i.outStack->location;
  ASSERT(key1.isInt() && key2.isString());

  TRACE(1, "nvGet2 outLoc: (%s, %lld)\n", outLoc.spaceName(), outLoc.offset);
  if (false) { // typeCheck
    ArrayData *a = NULL;
    TypedValue tv;
    StringData *sd = NULL;
    array_getm_is(a, 666, sd, &tv);
    array_getm_is0(a, 666, sd, &tv);
  }
  void* fptr;
  // array_getm_is incRefs the return value if appropriate and
  // it decRefs the string key for us. We don't need to decRef
  // the array because it was passed via a local.
  bool decRefStrKey =
    key2.isStack() && key2.rtt.valueString() == NULL;
  fptr = decRefStrKey ? ((void*)array_getm_is) : ((void*)array_getm_is0);

  int args[3];
  args[0] = array.isVariant() ? ArgAnyReg : 0;
  args[1] = 1;
  args[2] = 2;
  allocInputsForCall(i, args);

  EMIT_CALL(a, fptr,
            array.isVariant() ? DEREF(array.location) : V(array.location),
            V(key1.location),
            V(key2.location),
            A(outLoc));
  recordReentrantCall(i);
  m_regMap.invalidate(outLoc);
}

void TranslatorX64::translateCGetM_GE(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  const int nameIdx = 0;
  const int keyIdx = 1;
  const DynLocation& name = *i.inputs[nameIdx];
  const DynLocation& key  = *i.inputs[keyIdx];
  const Location& outLoc  = i.outStack->location;

  emitGetGlobal(i, nameIdx, false /* allowCreate */);
  ScratchReg holdRax(m_regMap, rax);

  a.  test_reg64_reg64(rax, rax);
  DiamondReturn noGlobalRet;
  {
    UnlikelyIfBlock<CC_Z> ifNoGlobal(a, astubs, &noGlobalRet);
    if (const StringData* name = i.inputs[nameIdx]->rtt.valueString()) {
      EMIT_CALL(astubs, raiseUndefVariable, IMM((uint64_t)name));
    } else {
      m_regMap.allocInputReg(i, nameIdx);
      PhysReg nameReg = getReg(i.inputs[nameIdx]->location);
      EMIT_CALL(astubs, raiseUndefVariable, R(nameReg));
    }
    recordReentrantStubCall(i);

    emitStoreNull(astubs, outLoc);
    m_regMap.invalidate(outLoc);
    // The DiamondReturn patches the jump here to return to the code
    // after this whole function.
  }

  emitIncRefGeneric(rax, 0);
  m_regMap.allocInputReg(i, keyIdx, argNumToRegName[1]);
  // We're going to be making a function call in both branches so
  // let's only emit the spilling code once.
  emitCallSaveRegs();
  a.    cmp_imm32_disp_reg32(KindOfArray, TVOFF(m_type), rax);
  DiamondReturn notArrayRet;
  {
    UnlikelyIfBlock<CC_NZ> ifNotArray(a, astubs, &notArrayRet);
    ASSERT(key.isString() || key.isInt());
    void* fptr =
      key.isString() ? (void*)non_array_getm_s : (void*)non_array_getm_i;
    EMIT_CALL(astubs, (TCA)fptr, R(rax), V(key.location), A(outLoc));
    recordReentrantStubCall(i);
  }
  a.load_reg64_disp_reg64(rax, TVOFF(m_data), rax);
  emitArrayElem(i, &name /* XXX unused in emitArrayElem for GE */,
                rax, &key, outLoc);
}

static bool
isSupportedInstrVGetG(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  return (i.inputs[0]->rtt.isString());
}

void
TranslatorX64::analyzeVGetG(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = simplePlan(isSupportedInstrVGetG(i));
}

static TypedValue* lookupAddBoxedGlobal(StringData* name) {
  VarEnv* ve = g_vmContext->m_globalVarEnv;
  TypedValue* r = ve->lookupAdd(name);
  if (r->m_type != KindOfRef) {
    tvBox(r);
  }
  LITSTR_DECREF(name);
  return r;
}

void
TranslatorX64::translateVGetG(const Tracelet& t,
                              const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  ASSERT(i.outStack);
  ASSERT(i.outStack->isVariant());
  ASSERT(i.inputs[0]->location == i.outStack->location);

  using namespace TargetCache;
  const StringData* maybeName = i.inputs[0]->rtt.valueString();
  if (!maybeName) {
    EMIT_CALL(a, lookupAddBoxedGlobal, V(i.inputs[0]->location));
    recordCall(i);
  } else {
    CacheHandle ch = BoxedGlobalCache::alloc(maybeName);

    if (false) { // typecheck
      StringData *key = NULL;
      TypedValue UNUSED *glob = BoxedGlobalCache::lookupCreate(ch, key);
    }
    SKTRACE(1, i.source, "ch %d\n", ch);
    EMIT_CALL(a, BoxedGlobalCache::lookupCreate,
               IMM(ch),
               V(i.inputs[0]->location));
    recordCall(i);
  }
  m_regMap.bind(rax, i.outStack->location, KindOfRef, RegInfo::DIRTY);
  emitIncRefGeneric(rax, 0);
  emitDeref(a, rax, rax);
}

void
TranslatorX64::translateCGetM(const Tracelet& t,
                              const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  ASSERT(i.outStack);

  if (isSupportedCGetMProp(i)) {
    translateCGetMProp(t, i);
    return;
  }
  if (isSupportedCGetM_LEE(i)) {
    translateCGetM_LEE(t, i);
    return;
  }
  if (isSupportedCGetM_GE(i)) {
    translateCGetM_GE(t, i);
    return;
  }
  if (isSupportedCGetM_LE(i) || isSupportedCGetM_RE(i)) {
    const DynLocation& base = *i.inputs[0];
    const DynLocation& key  = *i.inputs[1];
    int args[2];
    args[0] = base.isVariant() ? ArgAnyReg : 0;
    args[1] = 1;
    allocInputsForCall(i, args);

    PhysReg baseReg = getReg(base.location);
    LazyScratchReg baseScratch(m_regMap);
    if (base.isVariant()) {
      baseScratch.alloc();
      emitDeref(a, baseReg, *baseScratch);
      baseReg = *baseScratch;
    }
    emitArrayElem(i, &base, baseReg, &key, i.outStack->location);
    return;
  }
  TRANSLATE_MINSTR_GENERIC(CGet, t, i);
}

void TranslatorX64::analyzeVGetM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = Interp;
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

void TranslatorX64::translateVGetM(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  TRANSLATE_MINSTR_GENERIC(VGet, t, ni);
}

void
TranslatorX64::analyzeIssetM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = simplePlan(
      ni.inputs.size() == 2 &&
      ni.immVec.locationCode() == LL &&
      ni.immVecM.size() == 1 &&
      mcodeMaybeArrayKey(ni.immVecM[0]) &&
      ni.inputs[0]->valueType() == KindOfArray &&
      (ni.inputs[1]->isString() || ni.inputs[1]->isInt())
    );
    return;
  }
  ni.m_txFlags = simpleOrSupportedPlan(
    ni.inputs.size() == 2 &&
    ni.immVec.locationCode() == LL &&
    ni.immVecM.size() == 1 &&
    mcodeMaybeArrayKey(ni.immVecM[0]) &&
    ni.inputs[0]->valueType() == KindOfArray &&
    (ni.inputs[1]->isString() || ni.inputs[1]->isInt())
  );
  if (!ni.isSimple()) {
    ASSERT(ni.isSupported());
    ni.manuallyAllocInputs = true;
  }
}

void TranslatorX64::translateIssetMSimple(const Tracelet& t,
                                          const NormalizedInstruction& ni) {
  const DynLocation& base = *ni.inputs[0];
  const DynLocation& key  = *ni.inputs[1];

  PhysReg arrReg = getReg(base.location);

  ScratchReg scratch(m_regMap);
  if (base.isVariant()) {
    emitDeref(a, arrReg, *scratch);
    arrReg = *scratch;
    SKTRACE(1, ni.source, "loaded variant\n");
  }

  typedef uint64 (*HelperFunc)(const void* arr, StringData* sd);
  HelperFunc helper = NULL;
  if (key.isInt()) {
    helper = (HelperFunc)array_issetm_i;
  } else {
    bool doIntCheck = !ni.hasConstImm;
    if (key.isStack()) {
      // on the stack, we need to decref strings.
      helper = doIntCheck ? array_issetm_s : array_issetm_s_fast;
    } else {
      ASSERT(key.isLocal() || key.location.isLiteral());
      helper = doIntCheck ? array_issetm_s0 : array_issetm_s0_fast;
    }
  }
  ASSERT(helper);
  // The array helpers can reenter; need to sync state.
  EMIT_CALL(a, helper, R(arrReg), V(key.location));

  // We didn't bother allocating the single output reg above;
  // it lives in rax now.
  ASSERT(ni.outStack && !ni.outLocal);
  ASSERT(ni.outStack->outerType() == KindOfBoolean);
  m_regMap.bind(rax, ni.outStack->location, KindOfBoolean,
                RegInfo::DIRTY);
}

void TranslatorX64::translateIssetM(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  if (ni.isSimple()) {
    translateIssetMSimple(t, ni);
  } else {
    ASSERT(ni.isSupported());
    TRANSLATE_MINSTR_GENERIC(Isset, t, ni);
  }
}

void TranslatorX64::analyzeEmptyM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = Interp;
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

void TranslatorX64::translateEmptyM(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  TRANSLATE_MINSTR_GENERIC(Empty, t, ni);
}

/*
 * SetM. We care about two cases in particular: objects and arrays.
 */
static inline bool
isSupportedSetMArray(const NormalizedInstruction& i) {
  const ImmVector& iv = i.immVec;
  const LocationCode lcode = iv.locationCode();
  return
    i.immVecM.size() == 1 && lcode == LL &&
    mcodeMaybeArrayKey(i.immVecM[0]) &&
    i.inputs.size() == 3 &&
    (i.inputs[0]->isStack() || i.grouped) && // rhs
    i.inputs[1]->valueType() == KindOfArray && // base
    (i.inputs[2]->isInt() || i.inputs[2]->isString()); // key
}

bool
isSupportedSetMProp(const NormalizedInstruction& i) {
  if (i.inputs.size() != 3) return false;
  SKTRACE(2, i.source, "setM prop candidate: prop supported: %d, rtt %s\n",
          mcodeMaybePropName(i.immVecM[0]),
          i.inputs[2]->rtt.pretty().c_str());
  return isNormalPropertyAccess(i, 2, 1) && !curFunc()->isPseudoMain();
}

void
TranslatorX64::analyzeSetM(Tracelet& t, NormalizedInstruction& i) {
  // TODO: We could be more aggressive in the translation plans when the
  // lefthand side isn't refcounted.
  ASSERT(!(isSupportedSetMProp(i) && isSupportedSetMArray(i)));
  if (!RuntimeOption::EvalJitMGeneric) {
    i.m_txFlags = supportedPlan(isSupportedSetMProp(i) ||
                                isSupportedSetMArray(i));
    if (i.m_txFlags) {
      i.manuallyAllocInputs = true;
    }
    return;
  }
  i.m_txFlags = Supported;
  i.manuallyAllocInputs = true;
}

void
TranslatorX64::emitPropSet(const NormalizedInstruction& i,
                           const DynLocation& base,
                           const DynLocation& rhs,
                           PhysReg rhsReg,
                           PhysReg fieldAddr) {
  DataType rhsType = rhs.rtt.valueType();

  // Store rhs in the field
  emitTvSet(i, rhsReg, rhsType, fieldAddr, 0, i.outStack || rhs.isLocal());

  if (!base.isLocal()) {
    const int kBaseIdx = 1;
    ASSERT(i.inputs[kBaseIdx] == &base);
    m_regMap.allocInputReg(i, kBaseIdx);
    PhysReg baseReg = getReg(base.location);
    emitDecRef(i, baseReg, base.outerType());
  }
}

void
TranslatorX64::translateSetMProp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  using namespace TargetCache;
  ASSERT(i.inputs.size() == 3);

  const int kRhsIdx       = 0;
  const int kBaseIdx      = 1;
  const int kPropIdx      = 2;
  const DynLocation& val  = *i.inputs[kRhsIdx];
  const DynLocation& base = *i.inputs[kBaseIdx];
  const DynLocation& prop = *i.inputs[kPropIdx];
  const Slot propOffset   = getPropertyOffset(i, kPropIdx, 1);

  const Location& valLoc  = val.location;
  const Location& baseLoc = base.location;
  const Location& propLoc = prop.location;

  bool fastSet = propOffset != kInvalidSlot && i.immVec.locationCode() == LC;

  // We only combine a CGetL with a SetM if the SetM's value
  // is unused. Otherwise we'd need to do the same incRef
  // on the result of CGetL that CGetL already does for us.
  ASSERT(!i.outStack || !val.isLocal());
  // We can't get a variant unless we combined a CGetL
  ASSERT(val.isLocal() || !val.isVariant());

  bool decRefRhs = !i.outStack && !val.isLocal();

  int args[3];
  args[kRhsIdx] = fastSet ? ArgAnyReg : 3;
  args[kBaseIdx] = fastSet || base.isVariant() ? ArgAnyReg : 1;
  args[kPropIdx] = fastSet ? ArgDontAllocate : 2;
  allocInputsForCall(i, args);

  LazyScratchReg rhsTmp(m_regMap);
  PhysReg rhsReg = getReg(valLoc);
  if (val.isVariant()) {
    rhsTmp.alloc();
    emitDeref(a, rhsReg, *rhsTmp);
    rhsReg = *rhsTmp;
  }

  if (fastSet) {
    ASSERT(!base.isLocal() && !base.isVariant());
    Stats::emitInc(a, Stats::Tx64_PropSetFast);
    ScratchReg rField(m_regMap);
    a.lea_reg64_disp_reg64(getReg(baseLoc), int(propOffset), *rField);
    emitPropSet(i, base, val, rhsReg, *rField);
    decRefRhs = false;
  } else {
    Stats::emitInc(a, Stats::Tx64_PropSetSlow);
    bool useCtx = !isContextFixed();
    const StringData* name = prop.rtt.valueString();
    ASSERT(name == NULL || name->isStatic());

    CacheHandle ch;
    TargetCache::pcb_set_func_t setFn = propSetPrep(
      ch, useCtx ? name : propCacheName(name).get(),
      base.isLocal() || base.isVariant() ? BASE_LOCAL : BASE_CELL,
      useCtx ? DYN_CONTEXT : STATIC_CONTEXT,
      name ? STATIC_NAME : DYN_NAME);
    if (useCtx) {
      EMIT_CALL(a, setFn,
                 IMM(ch),
                 base.isVariant() ? DEREF(baseLoc) : V(baseLoc),
                 V(propLoc),
                 R(rhsReg),
                 IMM(val.rtt.valueType()),
                 R(rVmFp));
    } else {
      EMIT_CALL(a, setFn,
                 IMM(ch),
                 base.isVariant() ? DEREF(baseLoc) : V(baseLoc),
                 V(propLoc),
                 R(rhsReg),
                 IMM(val.rtt.valueType()));
    }
    recordReentrantCall(i);
    if (!base.isLocal() && base.isVariant()) {
      m_regMap.allocInputReg(i, kBaseIdx);
      emitDecRef(i, getReg(baseLoc), KindOfRef);
    }
    if (i.outStack || decRefRhs) {
      m_regMap.allocInputReg(i, kRhsIdx);
      rhsReg = getReg(valLoc);
    }
  }

  if (i.outStack) {
    ASSERT(!val.isVariant());
    m_regMap.cleanRegs(RegSet(rhsReg));
    m_regMap.invalidate(valLoc);
    m_regMap.bind(rhsReg, i.outStack->location,
                  val.valueType(), RegInfo::DIRTY);
  } else if (decRefRhs) {
    emitDecRef(i, rhsReg, val.outerType());
  }
}

// Check if val incref/decref is forced as a side effect of analysis
// converting the val from a stack input to a local input.  The conversion from
// stack input to local input is beneficial both because it avoids push/pop
// overhead and because it avoids incref/decref, but if the val is an array,
// the incref/decref omission is safe only if the val can't possibly alias the
// base or any intermediate results.
//
// This method handles only the single-dim case.
bool TranslatorX64::forceMValIncDec(const NormalizedInstruction& ni,
                                    const DynLocation& base,
                                    const DynLocation& val) const {
  ASSERT(base.valueType() == KindOfArray || val.valueType() == KindOfArray);
  if (val.isLocal() &&
      (val.location == base.location ||
       (val.isVariant() && base.isVariant() &&
        val.valueType() == base.valueType()))) {
    // We don't allow a local input unless we also folded a following pop.
    ASSERT(!ni.outStack);
    // We can't avoid the inc/dec on val in the case of:
    //   $a[...] = $a;
    return true;
  }
  return false;
}

// Same semantics as above, but this method handles the general case.
bool TranslatorX64::forceMValIncDec(const Tracelet& t,
                                    const NormalizedInstruction& ni,
                                    const MInstrInfo& mii) const {
  if (mii.valCount() == 1) {
    const DynLocation& val = *ni.inputs[0];
    if (val.valueType() != KindOfArray) {
      SKTRACE(2, ni.source, "%s false (val is non-array)\n", __func__);
      return false;
    }
    if (ni.immVecM.size() == 1) {
      const DynLocation& base = *ni.inputs[1];
      SKTRACE(2, ni.source, "%s %s\n",
              __func__, forceMValIncDec(ni, base, val) ? "true" : "false");
      return forceMValIncDec(ni, base, val);
    }
    ASSERT(ni.immVecM.size() > 1);
    if (val.isLocal()) {
      SKTRACE(2, ni.source, "%s true (val may alias one or more bases)\n",
              __func__, long(a.code.frontier));
      return true;
    }
    ASSERT(val.isStack());
    SKTRACE(2, ni.source, "%s false (val is on stack)\n",
            __func__, long(a.code.frontier));
    return false;
  }
  SKTRACE(2, ni.source, "%s false (no val)\n", __func__);
  return false;
}

void
TranslatorX64::translateSetMArray(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 3);
  const DynLocation& val = *i.inputs[0];
  const DynLocation& arr = *i.inputs[1];
  const DynLocation& key = *i.inputs[2];
  ASSERT(arr.isLocal());

  ASSERT(val.isLocal() || !val.isVariant());
  ASSERT(!i.outStack || !val.isLocal());

  bool forceIncDec = forceMValIncDec(i, arr, val);

  const Location valLoc = val.location;
  const Location arrLoc = arr.location;
  const Location keyLoc = key.location;

  // The array_setm helpers will decRef any old value that is
  // overwritten if appropriate. If copy-on-write occurs, it will also
  // incRef the new array and decRef the old array for us. Finally,
  // some of the array_setm helpers will decRef the key if it is a
  // string (for cases where the key is not a local), while others do
  // not (for cases where the key is a local).
  bool useBoxedForm = arr.isVariant();
  void* fptr;
  if (false) { // helper type-checks
    TypedValue* cell = NULL;
    ArrayData* arr = NULL;
    TypedValue* rhs = NULL;
    StringData* strKey = NULL;
    UNUSED ArrayData* ret = array_setm_ik1_iv(cell, arr, 12, 3);
    ret = array_setm_ik1_v(cell, arr, 12, rhs);
    ret = array_setm_sk1_v(cell, arr, strKey, rhs);
    ret = array_setm_sk1_v0(cell, arr, strKey, rhs);
    ret = array_setm_s0k1_v(cell, arr, strKey, rhs);
    ret = array_setm_s0k1_v0(cell, arr, strKey, rhs);
    ret = array_setm_s0k1nc_v(cell, arr, strKey, rhs);
    ret = array_setm_s0k1nc_v0(cell, arr, strKey, rhs);
  }
  bool isInt = key.isInt() && IS_INT_TYPE(val.rtt.valueType());

  /*
   * The value can be passed as A, V or DEREF according to:
   *
   *                       | val.isVariant() | !val.isVariant()
   * ----------------------+-----------------+-----------------------------
   * int key + int value   | DEREF(valLoc)   | V(valLoc)
   * everything else       | V(valLoc)       | A(valLoc)
   */
  int deRefLevel = isInt + val.isVariant();

  int args[3];
  args[0] = deRefLevel == 1 ? 3 : deRefLevel == 2 ? ArgAnyReg : ArgDontAllocate;
  args[1] = useBoxedForm ? 0 : 1;
  args[2] = 2;
  allocInputsForCall(i, args);

  if (isInt) {
    // If the key and rhs are Int64, we can use a specialized helper
    fptr = (void*)array_setm_ik1_iv;
  } else {
    bool decRefKey = key.rtt.isString() && keyLoc.isStack();
    bool decRefValue = forceIncDec ||
      (!i.outStack && !val.isLocal() &&
       IS_REFCOUNTED_TYPE(val.rtt.valueType()));
    fptr = decRefValue ?
      (key.rtt.isString() ?
       (decRefKey ? (void*)array_setm_sk1_v0 :
        (i.hasConstImm ? (void*)array_setm_s0k1nc_v0 :
         (void*)array_setm_s0k1_v0)) :
       (void*)array_setm_ik1_v0) :
      (key.rtt.isString() ?
       (decRefKey ? (void*)array_setm_sk1_v :
        (i.hasConstImm ? (void*)array_setm_s0k1nc_v :
         (void*)array_setm_s0k1_v)) :
       (void*)array_setm_ik1_v);
  }

  if (forceIncDec) {
    LazyScratchReg tmp(m_regMap);
    PhysReg rhsReg = getReg(valLoc);
    if (val.isVariant()) {
      tmp.alloc();
      emitDeref(a, rhsReg, *tmp);
      rhsReg = *tmp;
    }
    emitIncRef(rhsReg, KindOfArray);
  }
  EMIT_CALL(a, fptr,
            useBoxedForm ? V(arrLoc) : IMM(0),
            useBoxedForm ? DEREF(arrLoc) : V(arrLoc),
            V(keyLoc),
            deRefLevel == 2 ? DEREF(valLoc) :
            deRefLevel == 1 ? V(valLoc) : A(valLoc));

  recordReentrantCall(i);
  // If we did not used boxed form, we need to tell the register allocator
  // to associate rax with arrLoc
  if (!useBoxedForm) {
    // The array_setm helper returns the up-to-date array pointer in rax.
    // Therefore, we can bind rax to arrLoc and mark it as dirty.
    m_regMap.markAsClean(arrLoc);
    m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
  }

  if (i.outStack) {
    ASSERT(!val.isVariant());
    // Bring the output value into its correct location on the stack.
    // Note that this has to happen after all of the above, since it
    // needs to read the key from this location.
    m_regMap.allocInputReg(i, 0);
    m_regMap.bind(getReg(valLoc), i.outStack->location,
                  val.outerType(), RegInfo::DIRTY);
  }
}

void
TranslatorX64::translateSetM(const Tracelet& t,
                             const NormalizedInstruction& i) {
  if (isSupportedSetMProp(i)) {
    translateSetMProp(t, i);
    return;
  }
  if (isSupportedSetMArray(i)) {
    translateSetMArray(t, i);
    return;
  }
  TRANSLATE_MINSTR_GENERIC(Set, t, i);
}

void TranslatorX64::analyzeSetOpM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = Interp;
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

void TranslatorX64::translateSetOpM(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  TRANSLATE_MINSTR_GENERIC(SetOp, t, ni);
}

void TranslatorX64::analyzeIncDecM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = Interp;
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

void TranslatorX64::translateIncDecM(const Tracelet& t,
                                     const NormalizedInstruction& ni) {
  TRANSLATE_MINSTR_GENERIC(IncDec, t, ni);
}

void
TranslatorX64::analyzeUnsetM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = Interp;
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

void
TranslatorX64::translateUnsetM(const Tracelet& t,
                               const NormalizedInstruction& ni) {
  TRANSLATE_MINSTR_GENERIC(Unset, t, ni);
}

void TranslatorX64::analyzeBindM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = Interp;
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

void TranslatorX64::translateBindM(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  TRANSLATE_MINSTR_GENERIC(Bind, t, ni);
}
}}}
