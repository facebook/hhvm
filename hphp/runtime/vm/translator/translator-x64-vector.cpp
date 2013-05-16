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

#include "hphp/runtime/base/complex_types.h"
#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/translator/translator-deps.h"
#include "hphp/runtime/vm/translator/translator-inline.h"
#include "hphp/runtime/vm/translator/translator-runtime.h"
#include "hphp/runtime/vm/translator/translator-x64.h"
#include "hphp/runtime/vm/member_operations.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/shared/shared_map.h"

#include "hphp/runtime/vm/translator/translator-x64-internal.h"

#include <memory>

namespace HPHP {
namespace Transl {

/*
 * Translator for vector instructions.
 */

struct MVecTransState {
  MVecTransState()
    : baseType(KindOfUninit)
    , needsMIS(true)
  {}

  bool isKnown() { return baseType != KindOfUninit; }
  bool isObj() { return baseType == KindOfObject; }
  void setObj() { baseType = KindOfObject; }
  void resetBase() { baseType = KindOfUninit; }
  bool needsMIState() { return needsMIS; }
  void setNoMIState() { needsMIS = false; }

 private:
  /* This stores the type of the current value in rBase. If it's !=
   * KindOfUninit then the register will hold the value itself instead of a
   * TypedValue* */
  DataType baseType;
  /* Many vector instructions can be executed without using an MInstrState
   * struct - so we avoid the memory traffic in those cases. */
  bool needsMIS;
};

/* To ensure that we don't emit code to access an MInstrState struct when we
 * won't have one on the stack, access to rsp and offsets within MInstrState is
 * always done through these macros. */
template<typename T>
static inline T misAssertHelper(T value, bool condition) {
  assert(condition);
  return value;
}

#undef MISOFF
#define MISOFF(memb) \
  misAssertHelper(offsetof(MInstrState, memb), m_vecState->needsMIState())
static const PhysReg unsafe_rsp = rsp;
#define rsp
#define mis_rsp misAssertHelper(unsafe_rsp, m_vecState->needsMIState())

/* The next group of macros are used for passing a DynLocation as an argument
 * to a function (look for EMIT_CALL or EMIT_RCALL to see usage). Literals,
 * non-literal ints, non-literal strings, and generic TypedValues are all
 * supported. */

// TypedValue member location: fully generic
#define TVML(dl)                                                \
  (Stats::emitInc(a, Stats::Tx64_MTVKey), A(dl.location))
// Literal member location: takes a statement to use if the loc isn't a literal
#define LITML(dl, argElse)                                              \
  IE(dl.isLiteral(),                                                    \
     (Stats::emitInc(a, Stats::Tx64_MLitKey), IMM(dl.rtt.valueGeneric())), \
     argElse)
// Takes a location and a condition. If the condition is true the value of the
// location will be passed in a register
#define VALML(dl, cond)                                                 \
  IE((cond) && !dl.isRef(),                                             \
     (Stats::emitInc(a, Stats::Tx64_MRegKey),                           \
      m_regMap.allocInputReg(dl),                                       \
      V(dl.location)),                                                  \
     TVML(dl))
// Literals and non-literal strings
#define SML(dl)                                         \
  LITML(dl, VALML(dl, IS_STRING_TYPE(dl.valueType())))
// Literals and non-literal integers
#define IML(dl)                                         \
  LITML(dl, VALML(dl, dl.valueType() == KindOfInt64))
// Literals and non-literals ints and strings
#define ISML(dl)                                                        \
  LITML(dl, VALML(dl, dl.valueType() == KindOfInt64 ||                  \
                  IS_STRING_TYPE(dl.valueType())))

#define PREP_CTX(pr) Class* ctx = arGetContextClass(curFrame());

#define CTX() IMM(uintptr_t(ctx))

#define PREP_RESULT(useTvR)                                                   \
  if (!useTvR) {                                                              \
    m_regMap.cleanSmashLoc(result.location);                                  \
    ScratchReg rScratch(m_regMap);                                            \
    a.  lea_reg64_disp_reg64(rVmSp, vstackOffset(ni, mResultStackOffset(ni)), \
                             r(rScratch));                                    \
    emitStoreUninitNull(a, 0, r(rScratch));                                   \
  }
#define RESULT(useTvR)                            \
  IE((useTvR),                                    \
     RPLUS(mis_rsp, MISOFF(tvResult)),            \
     A(result.location))
#define PREP_VAL(useRVal, pr)                                       \
  LazyScratchReg rVal(m_regMap);                                    \
  if (useRVal) {                                                    \
    rVal.alloc(m_regMap.regIsFree(pr) ? pr : InvalidReg);           \
    PhysReg reg;                                                    \
    int disp;                                                       \
    locToRegDisp(val.location, &reg, &disp);                        \
    a.  loadq(reg[disp + TVOFF(m_data)], r(rVal));                  \
    if (auto offset = RefData::tvOffset()) a.addq(offset, r(rVal)); \
  }
#define VAL(useRVal)  \
  IE((useRVal),       \
     R(rVal),         \
     A(val.location))

static const MInstrAttr Warn = MIA_warn;
static const MInstrAttr Unset = MIA_unset;
static const MInstrAttr Reffy = MIA_reffy;
static const MInstrAttr Define = MIA_define;
static const MInstrAttr None = MIA_none;
static const MInstrAttr WarnDefine = MInstrAttr(Warn | Define);
static const MInstrAttr DefineReffy = MInstrAttr(Define | Reffy);
static const MInstrAttr WarnDefineReffy = MInstrAttr(Warn | Define | Reffy);
#define WDU(attrs) attrs & Warn, attrs & Define, attrs & Unset
#define WDRU(attrs) attrs & Warn, attrs & Define, attrs & Reffy, attrs & Unset

/* The next bunch of macros and functions are used to build up tables of helper
 * function pointers and determine which helper should be called based on a
 * variable number of bool and enum arguments. */

template<typename T> constexpr unsigned bitWidth() {
  static_assert(IncDec_invalid == 4,
                "IncDecOp enum must fit in 2 bits");
  static_assert(SetOp_invalid == 11,
                "SetOpOp enum must fit in 4 bits");
  return std::is_same<T, bool>::value ? 1
    : std::is_same<T, KeyType>::value ? 2
    : std::is_same<T, MInstrAttr>::value ? 4
    : std::is_same<T, IncDecOp>::value ? 2
    : std::is_same<T, SetOpOp>::value ? 4
    : sizeof(T) * CHAR_BIT;
}

// Determines the width in bits of all of its arguments
template<typename... T> unsigned multiBitWidth();
template<typename T, typename... Args>
inline unsigned multiBitWidth(T t, Args... args) {
  return bitWidth<T>() + multiBitWidth<Args...>(args...);
}
template<>
inline unsigned multiBitWidth() {
  return 0;
}

// Given the same arguments as multiBitWidth, buildBitmask will determine which
// index in the table corresponds to the provided parameters.
template<unsigned bit>
inline unsigned buildBitmask() {
  static_assert(bit < (sizeof(unsigned) * CHAR_BIT - 1), "Too many bits");
  return 0;
}
template<unsigned bit = 0, typename T, typename... Args>
inline unsigned buildBitmask(T c, Args... args) {
  unsigned bits = (unsigned)c & ((1u << bitWidth<T>()) - 1);
  return buildBitmask<bit + bitWidth<T>()>(args...) | bits << bit;
}

#define FILL_ROW(nm, ...) do {                          \
    OpFunc* dest = &optab[buildBitmask(__VA_ARGS__)];   \
    assert(*dest == nullptr);                           \
    *dest = nm;                                         \
  } while (false);
#define FILL_ROWH(nm, hot, ...) FILL_ROW(nm, __VA_ARGS__)

#define BUILD_OPTAB(...) BUILD_OPTAB_ARG(HELPER_TABLE(FILL_ROW), __VA_ARGS__)
#define BUILD_OPTABH(...) BUILD_OPTAB_ARG(HELPER_TABLE(FILL_ROWH), __VA_ARGS__)
#define BUILD_OPTAB_ARG(FILL_TABLE, ...)                                \
  static OpFunc* optab = nullptr;                                          \
  if (!optab) {                                                         \
    optab = (OpFunc*)calloc(1 << multiBitWidth(__VA_ARGS__), sizeof(OpFunc)); \
    FILL_TABLE                                                          \
  }                                                                     \
  unsigned idx = buildBitmask(__VA_ARGS__);                             \
  OpFunc opFunc = optab[idx];                                           \
  always_assert(opFunc);

// The getKeyType family of functions determine the KeyType to be used as a
// template argument to helper functions. S, IS, or I at the end of the
// function names signals that the caller supports non-literal strings, int, or
// both, respectively.
static KeyType getKeyType(const DynLocation& dl, bool nonLitStr,
                          bool nonLitInt) {
  if (dl.isRef()) {
    // Variants can change types at arbitrary times, so don't try to
    // pass them in registers.
    return AnyKey;
  }
  if ((dl.isLiteral() || nonLitStr) && IS_STRING_TYPE(dl.valueType())) {
    return StrKey;
  } else if ((dl.isLiteral() || nonLitInt) && dl.valueType() == KindOfInt64) {
    return IntKey;
  } else {
    assert(dl.isStack() || dl.isLocal());
    return AnyKey;
  }
}
inline static KeyType getKeyType(const DynLocation& dl) {
  return getKeyType(dl, false, false);
}
inline static KeyType getKeyTypeS(const DynLocation& dl) {
  return getKeyType(dl, true, false);
}
inline static KeyType getKeyTypeIS(const DynLocation& dl) {
  return getKeyType(dl, true, true);
}

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
    // Some instruction sequences, e.g. CGetL..SetM..PopC are optimized during
    // analysis to avoid the push/pop, in which case the input is a local
    // instead of a stack, and no val results from executing the VM instruction.
    if (input.isStack() && ni.outStack != nullptr) {
      return true;
    }
  } else if (mii.instr() == MI_IncDecM) {
    if (ni.outStack != nullptr) {
      return true;
    }
  }
  return false;
}

bool TranslatorX64::inputIsLiveForFinalOp(const NormalizedInstruction& ni,
                                        unsigned i,
                                        const MInstrInfo& mii) const {
  // It might be live if it's the final input (the last key)
  if (i == ni.inputs.size() - 1) {
    return true;
  }

  // Or if this is a SetOp and it's the first input
  if (mii.instr() == MI_SetOpM && i == 0) {
    return true;
  }

  return false;
}

int TranslatorX64::firstDecrefInput(const Tracelet& t,
                                    const NormalizedInstruction& ni,
                                    const MInstrInfo& mii) const {
  return logicalTeleportMVal(t, ni, mii) ? 1 : 0;
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
  if (&output == nullptr) {
    SKTRACE(2, ni.source, "%s (no stack output) --> false\n", __func__);
    return false;
  }
  assert(output.location.isStack());
  if (mii.instr() == MI_SetM || mii.instr() == MI_BindM) {
    SKTRACE(2, ni.source, "%s (%s val) --> false\n",
            __func__, mii.instr() == MI_SetM ? "SetM" : "BindM");
    return false;
  }

  int bottomI = 0;
  const DynLocation* bottomStack = nullptr;
  for (unsigned i = firstDecrefInput(t, ni, mii); i < ni.inputs.size(); ++i) {
    const DynLocation& input = *ni.inputs[i];
    // Only the bottommost stack input can possibly overlay the
    // output, but that might not be the first stack input in
    // ni.inputs. Walk through all stack inputs to find the bottommost
    // one.
    if (input.location.isStack() &&
        (!bottomStack ||
         input.location.offset < bottomStack->location.offset)) {
      bottomStack = &input;
      bottomI = i;
    }
  }
  if (bottomStack) {
    assert(bottomStack->location == output.location);
    // Use tvResult if the overlaid input is refcounted, or if it
    // might be used during the final operation (in which case it may
    // still be in use at the time the result is written).
    bool result = IS_REFCOUNTED_TYPE(bottomStack->outerType())
      || inputIsLiveForFinalOp(ni, bottomI, mii);
    SKTRACE(2, ni.source, "%s input %u/[0..%u) --> %s\n",
            __func__, bottomI, ni.inputs.size(), result ? "true" : "false");
    return result;
  }
  // No stack inputs. If we were to write the result directly to the VM stack,
  // it would potentially be clobbered by reentry during cleanup, e.g. for
  // ArrayAccess-related destruction. For now we make the very conservative
  // assumption that any instruction with statically unknown offsets can
  // reenter.
  bool result = mInstrHasUnknownOffsets(ni, curFunc()->cls());
  SKTRACE(2, ni.source, "%s (no stack inputs) --> %s\n",
          __func__, result ? "true" : "false");
  return result;
}

void TranslatorX64::emitBaseLCR(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                const MInstrInfo& mii, unsigned iInd,
                                LazyScratchReg& rBase) {
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

  const bool canEnregisterObj =
    base.isObject() &&
    mcodeMaybePropName(ni.immVecM[0]);

  if (canEnregisterObj) {
    m_regMap.allocInputReg(ni, iInd);
    auto curReg = getReg(base);
    m_regMap.cleanSmashReg(curReg);
    rBase.alloc(curReg);
    m_vecState->setObj();
  } else {
    PhysReg pr;
    int disp;
    locToRegDisp(base.location, &pr, &disp);
    rBase.alloc();

    m_regMap.cleanSmashLoc(base.location);
    if (base.isRef()) {
      // Get inner value.
      a.  loadq(pr[disp + TVOFF(m_data)], r(rBase));
      if (auto offset = RefData::tvOffset()) a.addq (offset, r(rBase));
    } else {
      a.  lea_reg64_disp_reg64(pr, disp, r(rBase));
    }
  }
}

void TranslatorX64::emitBaseH(unsigned iInd, LazyScratchReg& rBase) {
  m_regMap.allocInputReg(*m_curNI, iInd);
  const Location& l = m_curNI->inputs[iInd]->location;
  PhysReg r = getReg(l);
  m_regMap.smashReg(r);
  rBase.alloc(r);
  m_vecState->setObj();
}

template <bool warn, bool define>
static inline TypedValue* baseNImpl(TypedValue* key,
                                    MInstrState* mis,
                                    ActRec* fp) {
  TypedValue* base;
  StringData* name = prepareKey(key);
  const Func* func = fp->m_func;
  Id id = func->lookupVarId(name);
  if (id != kInvalidId) {
    base = frame_local(fp, id);
  } else {
    assert(!fp->hasInvName());
    if (define) {
      if (fp->m_varEnv == nullptr) {
        fp->m_varEnv = VarEnv::createLazyAttach(fp);
      }
      base = fp->m_varEnv->lookup(name);
      if (base == nullptr) {
        if (warn) {
          raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
        }
        TypedValue tv;
        tvWriteNull(&tv);
        fp->m_varEnv->set(name, &tv);
        base = fp->m_varEnv->lookup(name);
      }
    } else {
      if (fp->m_varEnv == nullptr || (base = fp->m_varEnv->lookup(name)) == nullptr) {
        if (warn) {
          raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
        }
        tvWriteNull(&mis->tvScratch);
        base = &mis->tvScratch;
      }
    }
  }
  decRefStr(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

static TypedValue* baseN(TypedValue* key, MInstrState* mis) {
  DECLARE_FRAME_POINTER(framePtr);
  return baseNImpl<false, false>(key, mis, (ActRec*)framePtr->m_savedRbp);
}

static TypedValue* baseNW(TypedValue* key, MInstrState* mis) {
  DECLARE_FRAME_POINTER(framePtr);
  return baseNImpl<true, false>(key, mis, (ActRec*)framePtr->m_savedRbp);
}

static TypedValue* baseND(TypedValue* key, MInstrState* mis) {
  DECLARE_FRAME_POINTER(framePtr);
  return baseNImpl<false, true>(key, mis, (ActRec*)framePtr->m_savedRbp);
}

static TypedValue* baseNWD(TypedValue* key, MInstrState* mis) {
  DECLARE_FRAME_POINTER(framePtr);
  return baseNImpl<true, true>(key, mis, (ActRec*)framePtr->m_savedRbp);
}

void TranslatorX64::emitBaseN(const Tracelet& t,
                              const NormalizedInstruction& ni,
                              const MInstrInfo& mii, unsigned iInd,
                              LazyScratchReg& rBase) {
  LocationCode lCode = ni.immVec.locationCode();
  const MInstrAttr& mia = mii.getAttr(lCode);
  SKTRACE(2, ni.source, "%s %#lx %s%s\n",
          __func__, long(a.code.frontier),
          (mia & MIA_warn) ? "W" : "", (mia & MIA_define) ? "D" : "");
  typedef TypedValue* (*BaseNOp)(TypedValue*, MInstrState*);
  static_assert(MIA_warn == 0x1 && MIA_define == 0x2,
                "MIA_* bitmask values were not as expected");
  static const BaseNOp baseNOps[] = {baseN, baseNW, baseND, baseNWD};
  assert((mia & MIA_base) < sizeof(baseNOps)/sizeof(BaseNOp));
  BaseNOp baseNOp = baseNOps[mia & MIA_base];
  const DynLocation& base = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(base.location);
  EMIT_RCALL(a, ni, baseNOp, A(base.location), R(mis_rsp));
  rBase.alloc(rax);
}

template <bool warn, bool define>
static inline TypedValue* baseGImpl(TypedValue* key,
                                    MInstrState* mis) {
  TypedValue* base;
  StringData* name = prepareKey(key);
  VarEnv* varEnv = g_vmContext->m_globalVarEnv;
  assert(varEnv != nullptr);
  base = varEnv->lookup(name);
  if (base == nullptr) {
    if (warn) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    if (define) {
      TypedValue tv;
      tvWriteNull(&tv);
      varEnv->set(name, &tv);
      base = varEnv->lookup(name);
    } else {
      tvWriteNull(&mis->tvScratch);
      base = &mis->tvScratch;
    }
  }
  decRefStr(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

static TypedValue* baseG(TypedValue* key, MInstrState* mis) {
  return baseGImpl<false, false>(key, mis);
}

static TypedValue* baseGW(TypedValue* key, MInstrState* mis) {
  return baseGImpl<true, false>(key, mis);
}

static TypedValue* baseGD(TypedValue* key, MInstrState* mis) {
  return baseGImpl<false, true>(key, mis);
}

static TypedValue* baseGWD(TypedValue* key, MInstrState* mis) {
  return baseGImpl<true, true>(key, mis);
}

void TranslatorX64::emitBaseG(const Tracelet& t,
                              const NormalizedInstruction& ni,
                              const MInstrInfo& mii, unsigned iInd,
                              LazyScratchReg& rBase) {
  LocationCode lCode = ni.immVec.locationCode();
  const MInstrAttr& mia = mii.getAttr(lCode);
  SKTRACE(2, ni.source, "%s %#lx %s%s\n", __func__, long(a.code.frontier),
          (mia & MIA_warn) ? "W" : "", (mia & MIA_define) ? "D" : "");
  typedef TypedValue* (*BaseGOp)(TypedValue*, MInstrState*);
  static_assert(MIA_warn == 0x1 && MIA_define == 0x2,
                "MIA_* bitmask values were not as expected");
  static const BaseGOp baseGOps[] = {baseG, baseGW, baseGD, baseGWD};
  assert((mia & MIA_base) < sizeof(baseGOps)/sizeof(BaseGOp));
  BaseGOp baseGOp = baseGOps[mia & MIA_base];
  const DynLocation& base = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(base.location);
  EMIT_RCALL(a, ni, baseGOp, A(base.location), R(mis_rsp));
  rBase.alloc(rax);
}

static TypedValue* baseS(Class* ctx, TypedValue* key, const Class* cls,
                         MInstrState* mis) {
  TypedValue* base;
  StringData* name = prepareKey(key);
  bool visible, accessible;
  base = cls->getSProp(ctx, name, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(), name->data());
  }
  decRefStr(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

static TypedValue* baseSClsRef(Class* ctx, TypedValue* key,
                               TypedValue* clsRef,
                               MInstrState* mis) {
  assert(clsRef->m_type == KindOfClass);
  const Class* cls = clsRef->m_data.pcls;
  return baseS(ctx, key, cls, mis);
}

void TranslatorX64::emitBaseS(const Tracelet& t,
                              const NormalizedInstruction& ni, unsigned iInd,
                              LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx\n", __func__, long(a.code.frontier));
  const DynLocation& key = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(key.location);
  const int kClassIdx = ni.inputs.size() - 1;
  const DynLocation& clsRef = *ni.inputs[kClassIdx];
  assert(clsRef.valueType() == KindOfClass);
  const Class* cls = clsRef.rtt.valueClass();

  const bool uniqueKnownClass =
    cls != nullptr &&
    (cls->preClass()->attrs() & AttrUnique);
  const bool canUseCache =
    cls &&
    key.isString() &&
    key.rtt.valueString() &&
    curFunc()->cls() == cls;

  // Emit the appropriate helper call.
  if (canUseCache) {
    SKTRACE(3, ni.source, "%s using SPropCache\n", __func__);

    rBase.alloc();
    emitStaticPropInlineLookup(*m_curNI, kClassIdx, key, r(rBase));
  } else if (uniqueKnownClass && RuntimeOption::RepoAuthoritative) {
    SKTRACE(3, ni.source, "%s using uniqueKnownClass\n", __func__);

    PREP_CTX(argNumToRegName[0]);
    EMIT_RCALL(a, ni, baseS,
                      CTX(),
                      A(key.location),
                      IMM(uintptr_t(cls)),
                      R(mis_rsp));
    rBase.alloc(rax);
  } else {
    SKTRACE(3, ni.source, "%s using generic\n", __func__);

    PREP_CTX(argNumToRegName[0]);
    m_regMap.cleanSmashLoc(clsRef.location);
    EMIT_RCALL(a, ni, baseSClsRef,
                      CTX(),
                      A(key.location),
                      A(clsRef.location),
                      R(mis_rsp));
    rBase.alloc(rax);
  }
}

void TranslatorX64::emitBaseOp(const Tracelet& t,
                               const NormalizedInstruction& ni,
                               const MInstrInfo& mii, unsigned iInd,
                               LazyScratchReg& rBase) {
  LocationCode lCode = ni.immVec.locationCode();
  switch (lCode) {
  case LL: case LC: case LR: emitBaseLCR(t, ni, mii, iInd, rBase);    break;
  case LH:                   emitBaseH(iInd, rBase);                  break;
  case LGL: case LGC:        emitBaseG(t, ni, mii, iInd, rBase);      break;
  case LNL: case LNC:        emitBaseN(t, ni, mii, iInd, rBase);      break;
  case LSL: case LSC:        emitBaseS(t, ni, iInd, rBase); break;
  default:                   not_reached();
  }
}

template<KeyType kt, bool isRef>
static inline TypedValue* unbox(TypedValue* k) {
  if (isRef) {
    if (kt == AnyKey) {
      assert(k->m_type == KindOfRef);
      k = k->m_data.pref->tv();
      assert(k->m_type != KindOfRef);
    } else {
      assert(k->m_type == keyDataType(kt) ||
             (IS_STRING_TYPE(k->m_type) && IS_STRING_TYPE(keyDataType(kt))));
      return reinterpret_cast<TypedValue*>(k->m_data.num);
    }
  } else if (kt == AnyKey) {
    assert(k->m_type != KindOfRef);
  }
  return k;
}

template <KeyType keyType, bool unboxKey, bool warn, bool define, bool reffy,
          bool unset>
static inline TypedValue* elemImpl(TypedValue* base, TypedValue* key,
                                   MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  if (unset) {
    return ElemU<keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else if (define) {
    return ElemD<warn, reffy, keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else {
    return Elem<warn, keyType>(mis->tvScratch, mis->tvRef, base, key);
  }
}

#define HELPER_TABLE(m)                                               \
  /* name        hot        key     unboxKey  attrs  */               \
  m(elemC,     ,            AnyKey,  false,   None)                   \
  m(elemCD,    ,            AnyKey,  false,   Define)                 \
  m(elemCDR,   ,            AnyKey,  false,   DefineReffy)            \
  m(elemCU,    ,            AnyKey,  false,   Unset)                  \
  m(elemCW,    ,            AnyKey,  false,   Warn)                   \
  m(elemCWD,   ,            AnyKey,  false,   WarnDefine)             \
  m(elemCWDR,  ,            AnyKey,  false,   WarnDefineReffy)        \
  m(elemI,     ,            IntKey,  false,   None)                   \
  m(elemID,    ,            IntKey,  false,   Define)                 \
  m(elemIDR,   ,            IntKey,  false,   DefineReffy)            \
  m(elemIU,    ,            IntKey,  false,   Unset)                  \
  m(elemIW,    ,            IntKey,  false,   Warn)                   \
  m(elemIWD,   ,            IntKey,  false,   WarnDefine)             \
  m(elemIWDR,  ,            IntKey,  false,   WarnDefineReffy)        \
  m(elemL,     ,            AnyKey,  true,    None)                   \
  m(elemLD,    ,            AnyKey,  true,    Define)                 \
  m(elemLDR,   ,            AnyKey,  true,    DefineReffy)            \
  m(elemLU,    ,            AnyKey,  true,    Unset)                  \
  m(elemLW,    ,            AnyKey,  true,    Warn)                   \
  m(elemLWD,   ,            AnyKey,  true,    WarnDefine)             \
  m(elemLWDR,  ,            AnyKey,  true,    WarnDefineReffy)        \
  m(elemLI,    ,            IntKey,  true,    None)                   \
  m(elemLID,   ,            IntKey,  true,    Define)                 \
  m(elemLIDR,  ,            IntKey,  true,    DefineReffy)            \
  m(elemLIU,   ,            IntKey,  true,    Unset)                  \
  m(elemLIW,   ,            IntKey,  true,    Warn)                   \
  m(elemLIWD,  ,            IntKey,  true,    WarnDefine)             \
  m(elemLIWDR, ,            IntKey,  true,    WarnDefineReffy)        \
  m(elemLS,    ,            StrKey,  true,    None)                   \
  m(elemLSD,   ,            StrKey,  true,    Define)                 \
  m(elemLSDR,  ,            StrKey,  true,    DefineReffy)            \
  m(elemLSU,   ,            StrKey,  true,    Unset)                  \
  m(elemLSW,   ,            StrKey,  true,    Warn)                   \
  m(elemLSWD,  ,            StrKey,  true,    WarnDefine)             \
  m(elemLSWDR, ,            StrKey,  true,    WarnDefineReffy)        \
  m(elemS,     ,            StrKey,  false,   None)                   \
  m(elemSD,    ,            StrKey,  false,   Define)                 \
  m(elemSDR,   ,            StrKey,  false,   DefineReffy)            \
  m(elemSU,    ,            StrKey,  false,   Unset)                  \
  m(elemSW,    ,            StrKey,  false,   Warn)                   \
  m(elemSWD,   ,            StrKey,  false,   WarnDefine)             \
  m(elemSWDR,  ,            StrKey,  false,   WarnDefineReffy)

#define ELEM(nm, hot, keyType, unboxKey, attrs)                         \
hot                                                                     \
static TypedValue* nm(TypedValue* base, TypedValue* key,                \
                              MInstrState* mis) {        \
  return elemImpl<keyType, unboxKey, WDRU(attrs)>(base, key, mis);      \
}
HELPER_TABLE(ELEM)
#undef ELEM

void TranslatorX64::emitElem(const Tracelet& t,
                             const NormalizedInstruction& ni,
                             const MInstrInfo& mii, unsigned mInd,
                             unsigned iInd, LazyScratchReg& rBase) {
  MemberCode mCode = ni.immVecM[mInd];
  MInstrAttr mia = MInstrAttr(mii.getAttr(mCode) & MIA_intermediate);
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u flags %x\n",
          __func__, long(a.code.frontier), mInd, iInd, mia);
  const DynLocation& memb = *ni.inputs[iInd];

  typedef TypedValue* (*OpFunc)(TypedValue*, TypedValue*,
                                MInstrState*);
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isRef(), mia);
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(memb), R(mis_rsp));
  rBase.realloc(rax);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, MInstrAttr attrs, bool isObj>
static inline TypedValue* propImpl(Class* ctx, TypedValue* base,
                                   TypedValue* key,
                                   MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  return Prop<WDU(attrs), isObj, keyType>(
    mis->tvScratch, mis->tvRef, ctx, base, key);
}

#define HELPER_TABLE(m)                                              \
  /* name        key  unboxKey attrs        isObj */                 \
  m(propC,     AnyKey,  false, None,        false)                   \
  m(propCD,    AnyKey,  false, Define,      false)                   \
  m(propCDO,   AnyKey,  false, Define,       true)                   \
  m(propCO,    AnyKey,  false, None,         true)                   \
  m(propCU,    AnyKey,  false, Unset,       false)                   \
  m(propCUO,   AnyKey,  false, Unset,        true)                   \
  m(propCW,    AnyKey,  false, Warn,        false)                   \
  m(propCWD,   AnyKey,  false, WarnDefine,  false)                   \
  m(propCWDO,  AnyKey,  false, WarnDefine,   true)                   \
  m(propCWO,   AnyKey,  false, Warn,         true)                   \
  m(propL,     AnyKey,   true, None,        false)                   \
  m(propLD,    AnyKey,   true, Define,      false)                   \
  m(propLDO,   AnyKey,   true, Define,       true)                   \
  m(propLO,    AnyKey,   true, None,         true)                   \
  m(propLU,    AnyKey,   true, Unset,       false)                   \
  m(propLUO,   AnyKey,   true, Unset,        true)                   \
  m(propLW,    AnyKey,   true, Warn,        false)                   \
  m(propLWD,   AnyKey,   true, WarnDefine,  false)                   \
  m(propLWDO,  AnyKey,   true, WarnDefine,   true)                   \
  m(propLWO,   AnyKey,   true, Warn,         true)                   \
  m(propLS,    StrKey,   true, None,        false)                   \
  m(propLSD,   StrKey,   true, Define,      false)                   \
  m(propLSDO,  StrKey,   true, Define,       true)                   \
  m(propLSO,   StrKey,   true, None,         true)                   \
  m(propLSU,   StrKey,   true, Unset,       false)                   \
  m(propLSUO,  StrKey,   true, Unset,        true)                   \
  m(propLSW,   StrKey,   true, Warn,        false)                   \
  m(propLSWD,  StrKey,   true, WarnDefine,  false)                   \
  m(propLSWDO, StrKey,   true, WarnDefine,   true)                   \
  m(propLSWO,  StrKey,   true, Warn,         true)                   \
  m(propS,     StrKey,  false, None,        false)                   \
  m(propSD,    StrKey,  false, Define,      false)                   \
  m(propSDO,   StrKey,  false, Define,       true)                   \
  m(propSO,    StrKey,  false, None,         true)                   \
  m(propSU,    StrKey,  false, Unset,       false)                   \
  m(propSUO,   StrKey,  false, Unset,        true)                   \
  m(propSW,    StrKey,  false, Warn,        false)                   \
  m(propSWD,   StrKey,  false, WarnDefine,  false)                   \
  m(propSWDO,  StrKey,  false, WarnDefine,   true)                   \
  m(propSWO,   StrKey,  false, Warn,         true)

#define PROP(nm, ...)                                                   \
static TypedValue* nm(Class* ctx, TypedValue* base, TypedValue* key,    \
                              MInstrState* mis) {        \
  return propImpl<__VA_ARGS__>(ctx, base, key, mis);                    \
}
HELPER_TABLE(PROP)
#undef PROP

void TranslatorX64::emitPropGeneric(const Tracelet& t,
                                    const NormalizedInstruction& ni,
                                    const MInstrInfo& mii,
                                    unsigned mInd, unsigned iInd,
                                    LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  MemberCode mCode = ni.immVecM[mInd];
  MInstrAttr mia = MInstrAttr(mii.getAttr(mCode) & MIA_intermediate_prop);
  const DynLocation& memb = *ni.inputs[iInd];
  PREP_CTX(argNumToRegName[0]);
  // Emit the appropriate helper call.
  Stats::emitInc(a, Stats::PropAsm_Generic);
  typedef TypedValue* (*OpFunc)(Class*, TypedValue*, TypedValue*,
                                MInstrState*);
  BUILD_OPTAB(getKeyTypeS(memb), memb.isRef(), mia, m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(),
                    R(rBase),
                    SML(memb),
                    R(mis_rsp));
  rBase.realloc(rax);
  m_vecState->resetBase();
}
#undef HELPER_TABLE

PropInfo getPropertyOffset(const NormalizedInstruction& ni,
                           Class* ctx,
                           const Class*& baseClass,
                           const MInstrInfo& mii,
                           unsigned mInd, unsigned iInd) {
  if (mInd == 0) {
    auto const baseIndex = mii.valCount();
    baseClass = ni.inputs[baseIndex]->rtt.isObject()
      ? ni.inputs[baseIndex]->rtt.valueClass()
      : nullptr;
  } else {
    baseClass = ni.immVecClasses[mInd - 1];
  }
  if (!baseClass) return PropInfo();

  if (!ni.inputs[iInd]->rtt.isString()) {
    return PropInfo();
  }
  auto* const name = ni.inputs[iInd]->rtt.valueString();
  if (!name) return PropInfo();

  bool accessible;
  // If we are not in repo-authoriative mode, we need to check that
  // baseClass cannot change in between requests
  if (!RuntimeOption::RepoAuthoritative ||
      !(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return PropInfo();
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return PropInfo();
      }
    }
  }
  // Lookup the index of the property based on ctx and baseClass
  Slot idx = baseClass->getDeclPropIndex(ctx, name, accessible);
  // If we couldn't find a property that is accessible in the current
  // context, bail out
  if (idx == kInvalidSlot || !accessible) {
    return PropInfo();
  }
  // If it's a declared property we're good to go: even if a subclass
  // redefines an accessible property with the same name it's guaranteed
  // to be at the same offset
  return PropInfo(
    baseClass->declPropOffset(idx),
    baseClass->declPropHphpcType(idx)
  );
}

PropInfo getFinalPropertyOffset(const NormalizedInstruction& ni,
                                Class* context,
                                const MInstrInfo& mii) {
  unsigned mInd = ni.immVecM.size() - 1;
  unsigned iInd = mii.valCount() + 1 + mInd;

  const Class* cls = nullptr;
  return getPropertyOffset(ni, context, cls, mii, mInd, iInd);
}

void TranslatorX64::emitPropSpecialized(MInstrAttr const mia,
                                        const Class* baseClass,
                                        int propOffset,
                                        unsigned mInd,
                                        unsigned iInd,
                                        LazyScratchReg& rBase) {
  SKTRACE(2, m_curNI->source, "%s class=%s offset=%d\n",
    __func__, baseClass->nameRef()->data(), propOffset);

  assert(!(mia & MIA_warn) || !(mia & MIA_unset));
  const bool doWarn   = mia & MIA_warn;
  const bool doDefine = mia & MIA_define || mia & MIA_unset;

  Stats::emitInc(a, Stats::PropAsm_Specialized);

  /*
   * Type-inference from hphpc only tells us that this either an
   * object of a given class type or null.  If it's not an object, it
   * has to be a null type based on type inference.  (It could be
   * KindOfRef with an object inside, except that this isn't inferred
   * for object properties so we're fine not checking KindOfRef in
   * that case.)
   *
   * On the other hand, if mInd == 0, we're operating on the base
   * which was already guarded by tracelet guards (and may have been
   * KindOfRef, but the Base* op already handled this).  So we only
   * need to do a type check against null here in the intermediate
   * cases.
   */
  std::unique_ptr<DiamondReturn> nonObjectRet;
  bool isObj = m_vecState->isObj();
  if (!isObj) {
    emitTypeCheck(a, KindOfObject, r(rBase), 0);
    {
      nonObjectRet.reset(new DiamondReturn());
      UnlikelyIfBlock ifNotObject(CC_NZ, a, astubs, nonObjectRet.get());
      if (doWarn) {
        EMIT_RCALL(astubs, *m_curNI, raisePropertyOnNonObject);
      }
      if (doDefine) {
        /*
         * NOTE:
         *
         * This case logically is supposed to do a stdClass promotion.
         * It should ideally not be possible (since we have a known
         * class type), except that the static compiler doesn't
         * correctly infer object class types in some edge cases
         * involving stdClass promotion.
         *
         * This is impossible to handle "correctly" if we're in the
         * middle of a multi-dim property expression, because things
         * further along may also have type inference telling them
         * that object properties are at a given slot, but the object
         * could actually be a stdClass instead of the knownCls type
         * if we were to promote here.
         *
         * So, we throw a fatal error, which is what hphpc's generated
         * C++ would do in this case too.
         *
         * Relevant TODOs:
         *   #1789661 (this can cause bugs if bytecode.cpp promotes)
         *   #1124706 (we want to get rid of stdClass promotion in general)
         */
        EMIT_RCALL(astubs, *m_curNI, throw_null_object_prop);
      } else {
        /*
         * This case is supposed to evaluate to null and let the vector
         * operation continue. However, the control flow to make that work
         * properly would be quite messy and this never happens in practice, so
         * throw a fatal instead of crashing.
         */
        EMIT_RCALL(astubs, *m_curNI, throw_null_get_object_prop);
      }
    }
  }

  ScratchReg rScratch(m_regMap);

  if (!isObj) {
    emitDeref(a, r(rBase), r(rBase));
  }
  a.    lea_reg64_disp_reg64(r(rBase), propOffset, r(rScratch));
  if (doWarn || doDefine) {
    emitCmpTVType(a, KindOfUninit, r(rScratch)[TVOFF(m_type)]);
    {
      UnlikelyIfBlock ifUninit(CC_Z, a, astubs);
      if (doWarn) {
        EMIT_RCALL(
          astubs, *m_curNI, raiseUndefProp,
          R(rBase),
          IMM(uintptr_t(m_curNI->inputs[iInd]->rtt.valueString()))
        );
      }
      if (doDefine) {
        emitStoreTVType(astubs, KindOfNull, r(rScratch)[TVOFF(m_type)]);
      } else {
        emitImmReg(astubs, uintptr_t(&init_null_variant), r(rScratch));
      }
    }
  }

  // We have to prevent ~ScratchReg from deallocating this Scratch, so
  // unbind it before doing the realloc.
  auto usedScratch = r(rScratch);
  rScratch.dealloc();
  rBase.realloc(usedScratch);
  m_vecState->resetBase();

  // nonObjectRet returns here.
}

void TranslatorX64::emitProp(const MInstrInfo& mii,
                             unsigned mInd, unsigned iInd,
                             LazyScratchReg& rBase) {
  SKTRACE(2, m_curNI->source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);

  const Class* knownCls = nullptr;
  const auto propInfo   = getPropertyOffset(*m_curNI, curFunc()->cls(),
                                            knownCls, mii,
                                            mInd, iInd);

  if (propInfo.offset == -1) {
    emitPropGeneric(*m_curTrace, *m_curNI, mii, mInd, iInd, rBase);
  } else {
    auto attrs = mii.getAttr(m_curNI->immVecM[mInd]);
    emitPropSpecialized(attrs, knownCls, propInfo.offset, mInd, iInd, rBase);
  }
}

static TypedValue* newElem(TypedValue* base, MInstrState* mis) {
  return NewElem(mis->tvScratch, mis->tvRef, base);
}

void TranslatorX64::emitNewElem(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                unsigned mInd, LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u\n",
          __func__, long(a.code.frontier), mInd);
  EMIT_RCALL(a, ni, newElem, R(rBase), R(mis_rsp));
  rBase.realloc(rax);
}

void TranslatorX64::emitIntermediateOp(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii,
                                       unsigned mInd, unsigned& iInd,
                                       LazyScratchReg& rBase) {
  switch (ni.immVecM[mInd]) {
  case MEC: case MEL: case MET: case MEI: {
    assert(!m_vecState->isKnown());
    emitElem(t, ni, mii, mInd, iInd, rBase);
    ++iInd;
    break;
  }
  case MPC: case MPL: case MPT:
    emitProp(mii, mInd, iInd, rBase);
    ++iInd;
    break;
  case MW:
    assert(!m_vecState->isKnown());
    assert(mii.newElem());
    emitNewElem(t, ni, mInd, rBase);
    break;
  default: not_reached();
  }
  /* If we consumed a key we're done with it now */
  if (ni.immVecM[mInd] != MW) {
    const DynLocation& key = *ni.inputs[iInd - 1];
    m_regMap.cleanSmashLoc(key.location);
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

  // If we've proven elsewhere that we don't need an MInstrState struct, we
  // know this translation won't need any ratchets
  if (!m_vecState->needsMIState()) return 0;

  unsigned ratchets = ni.immVecM.size();
  if (!needFirstRatchet(t, ni, mii)) --ratchets;
  if (!needFinalRatchet(t, ni, mii)) --ratchets;
  return ratchets;
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
                                    unsigned mInd,
                                    PhysReg rBase) {
  if (ratchetInd(t, ni, mii, mInd) < 0
      || ratchetInd(t, ni, mii, mInd) >= int(nLogicalRatchets(t, ni, mii))) {
    return;
  }
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, ratchetInd=%d/[0..%u)\n",
          __func__, long(a.code.frontier), mInd, ratchetInd(t, ni, mii, mInd),
          nLogicalRatchets(t, ni, mii));
  {
    UnlessUninit uu(a, mis_rsp, MISOFF(tvRef));
    // Clean up tvRef2 before overwriting it.
    if (ratchetInd(t, ni, mii, mInd) > 0) {
      emitDecRefGeneric(ni, mis_rsp, MISOFF(tvRef2));
    }
    // Copy tvRef to tvRef2.
    {
      ScratchReg rScratch(m_regMap);
      assert(sizeof(TypedValue) % 8 == 0);
      for (size_t off = 0; off < sizeof(TypedValue); off += 8) {
        a.load_reg64_disp_reg64(mis_rsp, MISOFF(tvRef) + off, r(rScratch));
        a.store_reg64_disp_reg64(r(rScratch), MISOFF(tvRef2) + off, mis_rsp);
      }
    }
    // Reset tvRef.
    emitStoreUninitNull(a, MISOFF(tvRef), mis_rsp);
    // Adjust base pointer.
    a.    lea_reg64_disp_reg64(mis_rsp, MISOFF(tvRef2), rBase);
  }
}
template <KeyType keyType, bool unboxKey, bool isObj>
static inline void cGetPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = Prop<true, false, false, isObj, keyType>(
    *result, mis->tvRef, ctx, base, key);
  if (base != result) {
    // Save a copy of the result.
    tvDup(base, result);
  }
  if (result->m_type == KindOfRef) {
    tvUnbox(result);
  }
}

#define HELPER_TABLE(m)                                       \
  /* name           hot        key  unboxKey  isObj */        \
  m(cGetPropC,    ,            AnyKey, false, false)          \
  m(cGetPropCO,   ,            AnyKey, false,  true)          \
  m(cGetPropL,    ,            AnyKey,  true, false)          \
  m(cGetPropLO,   ,            AnyKey,  true,  true)          \
  m(cGetPropLS,   ,            StrKey,  true, false)          \
  m(cGetPropLSO,  ,            StrKey,  true,  true)          \
  m(cGetPropS,    ,            StrKey, false, false)          \
  m(cGetPropSO,   ,            StrKey, false,  true)

#define PROP(nm, hot, ...)                                              \
hot                                                                     \
static void nm(Class* ctx, TypedValue* base, TypedValue* key,           \
                           TypedValue* result,                          \
                           MInstrState* mis) {           \
  cGetPropImpl<__VA_ARGS__>(ctx, base, key, result, mis);               \
}
HELPER_TABLE(PROP)
#undef PROP

void TranslatorX64::emitCGetProp(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii,
                                 unsigned mInd, unsigned iInd,
                                 LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  assert(!ni.outLocal);

  /*
   * If we know the class for the current base, emit a direct property
   * access.
   */
  const Class* knownCls = nullptr;
  const auto propInfo  = getPropertyOffset(*m_curNI, curFunc()->cls(),
                                           knownCls, mii, mInd, iInd);
  if (propInfo.offset != -1) {
    emitPropSpecialized(MIA_warn, knownCls, propInfo.offset,
                        mInd, iInd, rBase);
    emitDerefIfVariant(a, r(rBase));
    emitIncRefGeneric(r(rBase), 0);

    PhysReg stackOutReg;
    int stackOutDisp;
    if (useTvResult(t, ni, mii)) {
      stackOutReg = mis_rsp;
      stackOutDisp = MISOFF(tvResult);
    } else {
      locToRegDisp(ni.outStack->location, &stackOutReg, &stackOutDisp);
      invalidateOutStack(ni);
    }

    emitCopyToAligned(a, r(rBase), 0, stackOutReg, stackOutDisp);
    return;
  }

  Stats::emitInc(a, Stats::PropAsm_GenFinal);

  const DynLocation& memb = *ni.inputs[iInd];
  const DynLocation& result = *ni.outStack;
  PREP_CTX(argNumToRegName[0]);
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  // Emit the appropriate helper call.
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, TypedValue*,
                         MInstrState*);
  BUILD_OPTABH(getKeyTypeS(memb), memb.isRef(), m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc,
             CTX(), R(rBase), SML(memb), RESULT(useTvR), R(mis_rsp));
  if (!useTvR) invalidateOutStack(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey>
static inline void vGetElemImpl(TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = ElemD<false, true, keyType>(mis->tvScratch, mis->tvRef, base, key);
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

#define HELPER_TABLE(m)                                           \
  /* name          hot        keyType unboxKey */                 \
  m(vGetElemC,   ,            AnyKey,   false)                    \
  m(vGetElemI,   ,            IntKey,   false)                    \
  m(vGetElemL,   ,            AnyKey,    true)                    \
  m(vGetElemLI,  ,            IntKey,    true)                    \
  m(vGetElemLS,  ,            StrKey,    true)                    \
  m(vGetElemS,   ,            StrKey,   false)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
static void nm(TypedValue* base, TypedValue* key, TypedValue* result,   \
               MInstrState* mis) {                       \
  vGetElemImpl<__VA_ARGS__>(base, key, result, mis);                    \
}
HELPER_TABLE(ELEM)
#undef ELEM

void TranslatorX64::emitVGetElem(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, unsigned mInd,
                                 unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& memb = *ni.inputs[iInd];
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  typedef void (*OpFunc)(TypedValue*, TypedValue*, TypedValue*, MInstrState*);
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isRef());
  cleanOutLocal(ni);
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(memb), RESULT(useTvR), R(mis_rsp));
  invalidateOutLocal(ni);
  if (!useTvR) invalidateOutStack(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool isObj>
static inline void vGetPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = Prop<false, true, false, isObj, keyType>(mis->tvScratch, mis->tvRef,
                                                  ctx, base, key);
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

#define HELPER_TABLE(m)                                \
  /* name          hot        key   unboxKey isObj */  \
  m(vGetPropC,   ,            AnyKey, false, false)    \
  m(vGetPropCO,  ,            AnyKey, false,  true)    \
  m(vGetPropL,   ,            AnyKey,  true, false)    \
  m(vGetPropLO,  ,            AnyKey,  true,  true)    \
  m(vGetPropLS,  ,            StrKey,  true, false)    \
  m(vGetPropLSO, ,            StrKey,  true,  true)    \
  m(vGetPropS,   ,            StrKey, false, false)    \
  m(vGetPropSO,  ,            StrKey, false,  true)

#define PROP(nm, hot, ...)                                              \
hot                                                                     \
static void nm(Class* ctx, TypedValue* base, TypedValue* key,           \
               TypedValue* result,                                      \
               MInstrState* mis) {                       \
  vGetPropImpl<__VA_ARGS__>(ctx, base, key, result, mis);               \
}
HELPER_TABLE(PROP)
#undef PROP

void TranslatorX64::emitVGetProp(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii,
                                 unsigned mInd, unsigned iInd,
                                 LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& memb = *ni.inputs[iInd];
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  PREP_CTX(argNumToRegName[0]);
  // Emit the appropriate helper call.
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, TypedValue*,
                         MInstrState*);
  BUILD_OPTABH(getKeyTypeS(memb), memb.isRef(), m_vecState->isObj());
  cleanOutLocal(ni);
  EMIT_RCALL(a, ni, opFunc,
             CTX(), R(rBase), SML(memb), RESULT(useTvR), R(mis_rsp));
  invalidateOutLocal(ni);
  if (!useTvR) invalidateOutStack(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool useEmpty>
static inline bool issetEmptyElemImpl(TypedValue* base, TypedValue* key,
                                      MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  return IssetEmptyElem<useEmpty, false, keyType>(mis->tvScratch, mis->tvRef,
                                                  base, key);
}

#define HELPER_TABLE(m)                                      \
  /* name          hot        keyType unboxKey useEmpty */   \
  m(issetElemC,   ,            AnyKey, false,  false)        \
  m(issetElemCE,  ,            AnyKey, false,   true)        \
  m(issetElemI,   ,            IntKey, false,  false)        \
  m(issetElemIE,  ,            IntKey, false,   true)        \
  m(issetElemL,   ,            AnyKey,  true,  false)        \
  m(issetElemLE,  ,            AnyKey,  true,   true)        \
  m(issetElemLI,  ,            IntKey,  true,  false)        \
  m(issetElemLIE, ,            IntKey,  true,   true)        \
  m(issetElemLS,  ,            StrKey,  true,  false)        \
  m(issetElemLSE, ,            StrKey,  true,   true)        \
  m(issetElemS,   ,            StrKey, false,  false)        \
  m(issetElemSE,  ,            StrKey, false,   true)

#define ISSET(nm, hot, ...)                                     \
hot                                                             \
static bool nm(TypedValue* base, TypedValue* key,               \
               MInstrState* mis) {               \
  return issetEmptyElemImpl<__VA_ARGS__>(base, key, mis);       \
}
HELPER_TABLE(ISSET)
#undef ISSET

template <bool useEmpty>
void TranslatorX64::emitIssetEmptyElem(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii, unsigned mInd,
                                       unsigned iInd, PhysReg rBase) {
  const DynLocation& memb = *ni.inputs[iInd];
  typedef bool (*OpFunc)(TypedValue*, TypedValue*, MInstrState*);
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isRef(), useEmpty);
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(memb), R(mis_rsp));
  a.   and_imm32_reg64(1, rax); // Mask garbage bits.
  ScratchReg rIssetEmpty(m_regMap, rax);
  assert(!ni.outLocal);
  if (useTvResult(t, ni, mii)) {
    emitStoreTypedValue(a, KindOfBoolean, r(rIssetEmpty), MISOFF(tvResult),
                        mis_rsp);
  } else {
    m_regMap.bindScratch(rIssetEmpty, ni.outStack->location,
                         KindOfBoolean, RegInfo::DIRTY);
  }
}
#undef HELPER_TABLE

void TranslatorX64::emitIssetElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyElem<false>(t, ni, mii, mInd, iInd, rBase);
}

void TranslatorX64::emitEmptyElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyElem<true>(t, ni, mii, mInd, iInd, rBase);
}

template <KeyType keyType, bool unboxKey, bool useEmpty, bool isObj>
static inline bool issetEmptyPropImpl(Class* ctx, TypedValue* base,
                                      TypedValue* key,
                                      MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  return IssetEmptyProp<useEmpty, isObj, keyType>(ctx, base, key);
}

#define HELPER_TABLE(m)                                                 \
  /* name            hot        key  unboxKey useEmpty isObj */         \
  m(issetPropC,    ,            AnyKey, false, false,   false)          \
  m(issetPropCE,   ,            AnyKey, false,  true,   false)          \
  m(issetPropCEO,  ,            AnyKey, false,  true,    true)          \
  m(issetPropCO,   ,            AnyKey, false, false,    true)          \
  m(issetPropL,    ,            AnyKey,  true, false,   false)          \
  m(issetPropLE,   ,            AnyKey,  true,  true,   false)          \
  m(issetPropLEO,  ,            AnyKey,  true,  true,    true)          \
  m(issetPropLO,   ,            AnyKey,  true, false,    true)          \
  m(issetPropLS,   ,            StrKey,  true, false,   false)          \
  m(issetPropLSE,  ,            StrKey,  true,  true,   false)          \
  m(issetPropLSEO, ,            StrKey,  true,  true,    true)          \
  m(issetPropLSO,  ,            StrKey,  true, false,    true)          \
  m(issetPropS,    ,            StrKey, false, false,   false)          \
  m(issetPropSE,   ,            StrKey, false,  true,   false)          \
  m(issetPropSEO,  ,            StrKey, false,  true,    true)          \
  m(issetPropSO,   ,            StrKey, false, false,    true)

#define ISSET(nm, hot, ...)                                             \
hot                                                                     \
static bool nm(Class* ctx, TypedValue* base, TypedValue* key,           \
               MInstrState* mis) {                       \
  return issetEmptyPropImpl<__VA_ARGS__>(ctx, base, key, mis);          \
}
HELPER_TABLE(ISSET)
#undef ISSET

template <bool useEmpty>
void TranslatorX64::emitIssetEmptyProp(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii,
                                       unsigned mInd, unsigned iInd,
                                       PhysReg rBase) {
  const DynLocation& memb = *ni.inputs[iInd];
  PREP_CTX(argNumToRegName[0]);
  // Emit the appropriate helper call.
  typedef bool (*OpFunc)(Class* ctx, TypedValue*, TypedValue*, MInstrState*);
  BUILD_OPTABH(getKeyTypeS(memb), memb.isRef(), useEmpty,
              m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(), R(rBase), SML(memb));
  a.   andq(1, rax); // Mask garbage bits.
  ScratchReg rIssetEmpty(m_regMap, rax);
  assert(!ni.outLocal);
  if (useTvResult(t, ni, mii)) {
    emitStoreTypedValue(a, KindOfBoolean, r(rIssetEmpty), MISOFF(tvResult),
                        mis_rsp);
  } else {
    m_regMap.bindScratch(rIssetEmpty, ni.outStack->location, KindOfBoolean,
                         RegInfo::DIRTY);
  }
}
#undef HELPER_TABLE

void TranslatorX64::emitIssetProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii,
                                  unsigned mInd, unsigned iInd,
                                  LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyProp<false>(t, ni, mii, mInd, iInd, r(rBase));
}

void TranslatorX64::emitEmptyProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii,
                                  unsigned mInd, unsigned iInd,
                                  LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyProp<true>(t, ni, mii, mInd, iInd, r(rBase));
}

template <KeyType keyType, bool unboxKey, bool setResult>
static inline void setElemImpl(TypedValue* base, TypedValue* key, Cell* val) {
  key = unbox<keyType, unboxKey>(key);
  SetElem<setResult, keyType>(base, key, val);
}

#define HELPER_TABLE(m)                                        \
  /* name         hot        key   unboxKey setResult */       \
  m(setElemC,   ,            AnyKey, false, false)             \
  m(setElemCR,  ,            AnyKey, false,  true)             \
  m(setElemI,   ,            IntKey, false, false)             \
  m(setElemIR,  ,            IntKey, false,  true)             \
  m(setElemL,   ,            AnyKey,  true, false)             \
  m(setElemLR,  ,            AnyKey,  true,  true)             \
  m(setElemLI,  ,            IntKey,  true, false)             \
  m(setElemLIR, ,            IntKey,  true,  true)             \
  m(setElemLS,  ,            StrKey,  true, false)             \
  m(setElemLSR, ,            StrKey,  true,  true)             \
  m(setElemS,   ,            StrKey, false, false)             \
  m(setElemSR,  ,            StrKey, false,  true)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
static void nm(TypedValue* base, TypedValue* key, Cell* val) {          \
  setElemImpl<__VA_ARGS__>(base, key, val);                             \
}
HELPER_TABLE(ELEM)
#undef ELEM

void TranslatorX64::emitSetElem(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                const MInstrInfo& mii, unsigned mInd,
                                unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.cleanSmashLoc(val.location);
  bool useRVal = (!forceMValIncDec(t, ni, mii) && val.isRef());
  PREP_VAL(useRVal, argNumToRegName[2]);
  // Emit the appropriate helper call.
  typedef void (*OpFunc)(TypedValue*, TypedValue*, Cell*);
  BUILD_OPTABH(getKeyTypeIS(key), key.isRef(), setResult);
  cleanOutLocal(ni);
  EMIT_RCALL(a, ni, opFunc,
                    R(rBase),
                    ISML(key),
                    IE(forceMValIncDec(t, ni, mii),
                       RPLUS(mis_rsp, MISOFF(tvVal)),
                       VAL(useRVal)));
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool setResult, bool isObj>
static inline void setPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                               Cell* val) {
  key = unbox<keyType, unboxKey>(key);
  SetProp<setResult, isObj, keyType>(ctx, base, key, val);
}

#define HELPER_TABLE(m)                                       \
  /* name         key   unboxKey setResult isObj */           \
  m(setPropC,    AnyKey,  false,  false,   false)             \
  m(setPropCO,   AnyKey,  false,  false,    true)             \
  m(setPropCR,   AnyKey,  false,   true,   false)             \
  m(setPropCRO,  AnyKey,  false,   true,    true)             \
  m(setPropL,    AnyKey,   true,  false,   false)             \
  m(setPropLO,   AnyKey,   true,  false,    true)             \
  m(setPropLR,   AnyKey,   true,   true,   false)             \
  m(setPropLRO,  AnyKey,   true,   true,    true)             \
  m(setPropLS,   StrKey,   true,  false,   false)             \
  m(setPropLSO,  StrKey,   true,  false,    true)             \
  m(setPropLSR,  StrKey,   true,   true,   false)             \
  m(setPropLSRO, StrKey,   true,   true,    true)             \
  m(setPropS,    StrKey,  false,  false,   false)             \
  m(setPropSO,   StrKey,  false,  false,    true)             \
  m(setPropSR,   StrKey,  false,   true,   false)             \
  m(setPropSRO,  StrKey,  false,   true,    true)

#define PROP(nm, ...)                                                   \
static void nm(Class* ctx, TypedValue* base, TypedValue* key, Cell* val) { \
  setPropImpl<__VA_ARGS__>(ctx, base, key, val);                        \
}
HELPER_TABLE(PROP)
#undef PROP

void TranslatorX64::emitSetProp(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                const MInstrInfo& mii,
                                unsigned mInd, unsigned iInd,
                                LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const int kRhsIdx      = 0;
  const DynLocation& val = *ni.inputs[kRhsIdx];

  /*
   * If we know the class for the current base, emit a direct property
   * set.
   */
  const Class* knownCls = nullptr;
  const auto propInfo   = getPropertyOffset(*m_curNI, curFunc()->cls(),
                                            knownCls, mii, mInd, iInd);
  if (propInfo.offset != -1 && !ni.outLocal && !ni.outStack) {
    emitPropSpecialized(MIA_define, knownCls, propInfo.offset,
                        mInd, iInd, rBase);

    m_regMap.allocInputReg(*m_curNI, kRhsIdx);
    PhysReg rhsReg = getReg(val.location);
    LazyScratchReg tmp(m_regMap);
    if (val.isRef() && !IS_NULL_TYPE(val.rtt.valueType())) {
      tmp.alloc();
      emitDerefRef(a, rhsReg, r(tmp));
      rhsReg = r(tmp);
    }

    const bool incRef = true;
    emitTvSet(*m_curNI, rhsReg, val.rtt.valueType(), r(rBase), 0, incRef);
    return;
  }

  Stats::emitInc(a, Stats::PropAsm_GenFinal);

  const bool setResult   = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
        __func__, setResult ? "true" : "false");
  const DynLocation& key = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(val.location);
  PREP_CTX(argNumToRegName[0]);
  bool useRVal = val.isRef();
  PREP_VAL(useRVal, argNumToRegName[3]);
  // Emit the appropriate helper call.
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, Cell*);
  cleanOutLocal(ni);
  BUILD_OPTAB(getKeyTypeS(key), key.isRef(), setResult,
              m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(), R(rBase), SML(key), VAL(useRVal));
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, SetOpOp op, bool setResult>
static inline void setOpElemImpl(TypedValue* base, TypedValue* key, Cell* val,
                                 MInstrState* mis,
                                 TypedValue* tvRes=nullptr) {
  key = unbox<keyType, unboxKey>(key);
  TypedValue* result = SetOpElem<keyType>(mis->tvScratch, mis->tvRef, op, base,
                                          key, val);
  if (setResult) {
    tvReadCell(result, tvRes);
  }
}

#define OPELEM_TABLE(m, nm, op)                        \
  /* name            keyType unboxKey op setResult */  \
  m(nm##op##ElemC,   AnyKey, false,  op,  false)       \
  m(nm##op##ElemCR,  AnyKey, false,  op,   true)       \
  m(nm##op##ElemI,   IntKey, false,  op,  false)       \
  m(nm##op##ElemIR,  IntKey, false,  op,   true)       \
  m(nm##op##ElemL,   AnyKey,  true,  op,  false)       \
  m(nm##op##ElemLR,  AnyKey,  true,  op,   true)       \
  m(nm##op##ElemLI,  IntKey,  true,  op,  false)       \
  m(nm##op##ElemLIR, IntKey,  true,  op,   true)       \
  m(nm##op##ElemLS,  StrKey,  true,  op,  false)       \
  m(nm##op##ElemLSR, StrKey,  true,  op,   true)       \
  m(nm##op##ElemS,   StrKey, false,  op,  false)       \
  m(nm##op##ElemSR,  StrKey, false,  op,   true)

#define HELPER_TABLE(m, op) OPELEM_TABLE(m, setOp, SetOp##op)
#define SETOP(nm, ...)                                                  \
static void nm(TypedValue* base, TypedValue* key, Cell* val,            \
               MInstrState* mis,                         \
               TypedValue* tvRes) {                                     \
  setOpElemImpl<__VA_ARGS__>(base, key, val, mis, tvRes);               \
}
#define SETOP_OP(op, bcOp) HELPER_TABLE(SETOP, op)
SETOP_OPS
#undef SETOP_OP
#undef SETOP

void TranslatorX64::emitSetOpElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  SetOpOp op = SetOpOp(ni.imm[0].u_OA);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.cleanSmashLoc(val.location);
  bool useRVal = (!forceMValIncDec(t, ni, mii) && val.isRef());
  PREP_VAL(useRVal, argNumToRegName[2]);
  typedef void (*OpFunc)(TypedValue*, TypedValue*, Cell*, MInstrState*,
                         TypedValue*);
# define SETOP_OP(op, bcOp) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(SETOP_OPS, getKeyTypeIS(key), key.isRef(), op, setResult);
# undef SETOP_OP
  // Emit the appropriate helper call.
  cleanOutLocal(ni);
  if (setResult) {
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, opFunc,
                      R(rBase),
                      ISML(key),
                      IE(forceMValIncDec(t, ni, mii),
                         RPLUS(mis_rsp, MISOFF(tvVal)),
                         VAL(useRVal)),
                      R(mis_rsp),
                      RESULT(useTvR));
    if (!useTvR) invalidateOutStack(ni);
  } else {
    EMIT_RCALL(a, ni, opFunc,
                      R(rBase),
                      ISML(key),
                      IE(forceMValIncDec(t, ni, mii),
                         RPLUS(mis_rsp, MISOFF(tvVal)),
                         VAL(useRVal)),
                      R(mis_rsp));
  }
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, SetOpOp op, bool setResult,
          bool isObj>
static inline void setOpPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                 Cell* val, MInstrState* mis,
                                 TypedValue* tvRes=nullptr) {
  key = unbox<keyType, unboxKey>(key);
  TypedValue* result = SetOpProp<isObj, keyType>(mis->tvScratch, mis->tvRef,
                                                 ctx, op, base, key, val);
  if (setResult) {
    tvReadCell(result, tvRes);
  }
}

#define OPPROP_TABLE(m, nm, op)                                   \
  /* name             keyType unboxKey op setResult isObj */            \
  m(nm##op##PropC,    AnyKey,  false,  op, false,  false)               \
  m(nm##op##PropCO,   AnyKey,  false,  op, false,   true)               \
  m(nm##op##PropCR,   AnyKey,  false,  op,  true,  false)               \
  m(nm##op##PropCRO,  AnyKey,  false,  op,  true,   true)               \
  m(nm##op##PropL,    AnyKey,   true,  op, false,  false)               \
  m(nm##op##PropLO,   AnyKey,   true,  op, false,   true)               \
  m(nm##op##PropLR,   AnyKey,   true,  op,  true,  false)               \
  m(nm##op##PropLRO,  AnyKey,   true,  op,  true,   true)               \
  m(nm##op##PropLS,   StrKey,   true,  op, false,  false)               \
  m(nm##op##PropLSO,  StrKey,   true,  op, false,   true)               \
  m(nm##op##PropLSR,  StrKey,   true,  op,  true,  false)               \
  m(nm##op##PropLSRO, StrKey,   true,  op,  true,   true)               \
  m(nm##op##PropS,    StrKey,  false,  op, false,  false)               \
  m(nm##op##PropSO,   StrKey,  false,  op, false,   true)               \
  m(nm##op##PropSR,   StrKey,  false,  op,  true,  false)               \
  m(nm##op##PropSRO,  StrKey,  false,  op,  true,   true)

#define HELPER_TABLE(m, op) OPPROP_TABLE(m, setOp, SetOp##op)
#define SETOP(nm, ...)                                                  \
static void nm(Class* ctx, TypedValue* base, TypedValue* key,           \
               Cell* val, MInstrState* mis,              \
               TypedValue* tvRes) {                                     \
  setOpPropImpl<__VA_ARGS__>(ctx, base, key, val, mis, tvRes);          \
}
#define SETOP_OP(op, bcOp) HELPER_TABLE(SETOP, op)
SETOP_OPS
#undef SETOP_OP
#undef SETOP

void TranslatorX64::emitSetOpProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii,
                                  unsigned mInd, unsigned iInd,
                                  LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  SetOpOp op = SetOpOp(ni.imm[0].u_OA);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.cleanSmashLoc(val.location);
  PREP_CTX(argNumToRegName[0]);
  bool useRVal = val.isRef();
  bool isObj = m_vecState->isObj();
  PREP_VAL(useRVal, argNumToRegName[3]);
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, Cell*, MInstrState*,
                         TypedValue*);
# define SETOP_OP(op, bcOp) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(SETOP_OPS,
                  getKeyTypeS(key), key.isRef(), op, setResult, isObj);
# undef SETOP_OP
  // Emit the appropriate helper call.
  cleanOutLocal(ni);
  if (setResult) {
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, opFunc,
                      CTX(),
                      R(rBase),
                      SML(key),
                      VAL(useRVal),
                      R(mis_rsp),
                      RESULT(useTvR));
  if (!useTvR) invalidateOutStack(ni);
  } else {
    EMIT_RCALL(a, ni, opFunc,
               CTX(), R(rBase), SML(key), VAL(useRVal), R(mis_rsp));
  }
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, IncDecOp op, bool setResult>
static inline void incDecElemImpl(TypedValue* base, TypedValue* key,
                                  MInstrState* mis,
                                  TypedValue* tvRes) {
  key = unbox<keyType, unboxKey>(key);
  IncDecElem<setResult, keyType>(mis->tvScratch, mis->tvRef, op, base, key,
                                 *tvRes);
}

#define HELPER_TABLE(m, op) OPELEM_TABLE(m, incDec, op)
#define INCDEC(nm, ...)                                           \
static void nm(TypedValue* base, TypedValue* key,                 \
               MInstrState* mis,                   \
               TypedValue* tvRes) {                               \
  incDecElemImpl<__VA_ARGS__>(base, key, mis, tvRes);             \
}
#define INCDEC_OP(op) HELPER_TABLE(INCDEC, op)
INCDEC_OPS
#undef INCDEC_OP
#undef INCDEC

void TranslatorX64::emitIncDecElem(const Tracelet& t,
                                   const NormalizedInstruction& ni,
                                   const MInstrInfo& mii, unsigned mInd,
                                   unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  IncDecOp op = IncDecOp(ni.imm[0].u_OA);
  const DynLocation& key = *ni.inputs[iInd];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  typedef void (*OpFunc)(TypedValue*, TypedValue*, MInstrState*, TypedValue*);
# define INCDEC_OP(op) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(INCDEC_OPS, getKeyTypeIS(key), key.isRef(), op, setResult);
# undef INCDEC_OP
  // Emit the appropriate helper call.
  cleanOutLocal(ni);
  if (setResult) {
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(key), R(mis_rsp), RESULT(useTvR));
    if (!useTvR) invalidateOutStack(ni);
  } else {
    EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(key), R(mis_rsp));
  }
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, IncDecOp op, bool setResult,
          bool isObj>
static inline void incDecPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                  MInstrState* mis,
                                  TypedValue* tvRes) {
  key = unbox<keyType, unboxKey>(key);
  IncDecProp<setResult, isObj, keyType>(mis->tvScratch, mis->tvRef, ctx, op,
                                        base, key, *tvRes);
}


#define HELPER_TABLE(m, op) OPPROP_TABLE(m, incDec, op)
#define INCDEC(nm, ...)                                                 \
static void nm(Class* ctx, TypedValue* base, TypedValue* key,           \
               MInstrState* mis, TypedValue* tvRes) {    \
  incDecPropImpl<__VA_ARGS__>(ctx, base, key, mis, tvRes);              \
}
#define INCDEC_OP(op) HELPER_TABLE(INCDEC, op)
INCDEC_OPS
#undef INCDEC_OP
#undef INCDEC

void TranslatorX64::emitIncDecProp(const Tracelet& t,
                                   const NormalizedInstruction& ni,
                                   const MInstrInfo& mii,
                                   unsigned mInd, unsigned iInd,
                                   LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  IncDecOp op = IncDecOp(ni.imm[0].u_OA);
  const DynLocation& key = *ni.inputs[iInd];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  PREP_CTX(argNumToRegName[0]);
  bool isObj = m_vecState->isObj();
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, MInstrState*,
                         TypedValue*);
# define INCDEC_OP(op) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(INCDEC_OPS,
                  getKeyTypeS(key), key.isRef(), op, setResult, isObj);
# undef INCDEC_OP
  // Emit the appropriate helper call.
  cleanOutLocal(ni);
  if (setResult) {
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, opFunc,
                      CTX(),
                      R(rBase),
                      SML(key),
                      R(mis_rsp),
                      RESULT(useTvR));
    if (!useTvR) invalidateOutStack(ni);
  } else {
    EMIT_RCALL(a, ni, opFunc,
               CTX(), R(rBase), SML(key), R(mis_rsp));
  }
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey>
static inline void bindElemImpl(TypedValue* base, TypedValue* key, TypedValue* val,
                                MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = ElemD<false, true, keyType>(mis->tvScratch, mis->tvRef, base, key);
  assert(val->m_type == KindOfRef);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val, base);
  }
}

#define HELPER_TABLE(m)                         \
  /* name       keyType  unboxKey */            \
  m(bindElemC,   AnyKey,  false)                \
  m(bindElemI,   IntKey,  false)                \
  m(bindElemL,   AnyKey,   true)                \
  m(bindElemLI,  IntKey,   true)                \
  m(bindElemLS,  StrKey,   true)                \
  m(bindElemS,   StrKey,  false)

#define ELEM(nm, ...)                                                   \
static void nm(TypedValue* base, TypedValue* key, TypedValue* val,      \
               MInstrState* mis) {                                      \
  bindElemImpl<__VA_ARGS__>(base, key, val, mis);                       \
}
HELPER_TABLE(ELEM)
#undef ELEM

void TranslatorX64::emitBindElem(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, unsigned mInd,
                                 unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  assert(generateMVal(t, ni, mii));
  m_regMap.cleanSmashLoc(val.location);
  cleanOutLocal(ni);
  assert(!forceMValIncDec(t, ni, mii));
  assert(val.isRef());
  typedef void (*OpFunc)(TypedValue*, TypedValue*, TypedValue*, MInstrState*);
  BUILD_OPTAB(getKeyTypeIS(key), key.isRef());
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(key), A(val.location), R(mis_rsp));
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool isObj>
static inline void bindPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                TypedValue* val, MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = Prop<false, true, false, isObj, keyType>(mis->tvScratch, mis->tvRef,
                                                  ctx, base, key);
  assert(val->m_type == KindOfRef);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val, base);
  }
}

#define HELPER_TABLE(m)                    \
  /* name           key unboxKey isObj */  \
  m(bindPropC,    AnyKey, false, false)    \
  m(bindPropCO,   AnyKey, false,  true)    \
  m(bindPropL,    AnyKey,  true, false)    \
  m(bindPropLO,   AnyKey,  true,  true)    \
  m(bindPropLS,   StrKey,  true, false)    \
  m(bindPropLSO,  StrKey,  true,  true)    \
  m(bindPropS,    StrKey, false, false)    \
  m(bindPropSO,   StrKey, false,  true)

#define PROP(nm, ...)                                                   \
static inline void nm(Class* ctx, TypedValue* base, TypedValue* key,    \
                                TypedValue* val,                        \
                                MInstrState* mis) {                     \
  bindPropImpl<__VA_ARGS__>(ctx, base, key, val, mis);                  \
}
HELPER_TABLE(PROP)
#undef PROP

void TranslatorX64::emitBindProp(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii,
                                 unsigned mInd, unsigned iInd,
                                 LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);

  const DynLocation& val = *ni.inputs[0];
  m_regMap.cleanSmashLoc(val.location);
  assert(val.isRef());
  assert(generateMVal(t, ni, mii));
  const DynLocation& key = *ni.inputs[iInd];
  cleanOutLocal(ni);
  PREP_CTX(argNumToRegName[0]);
  // Emit the appropriate helper call.
  assert(!forceMValIncDec(t, ni, mii));
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, TypedValue*,
                         MInstrState*);
  BUILD_OPTAB(getKeyTypeS(key), key.isRef(), m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc,
             CTX(), R(rBase), SML(key), A(val.location), R(mis_rsp));
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey>
static inline void unsetElemImpl(TypedValue* base, TypedValue* key) {
  key = unbox<keyType, unboxKey>(key);
  UnsetElem<keyType>(base, key);
}

#define HELPER_TABLE(m)                                       \
  /* name           hot       keyType unboxKey */             \
  m(unsetElemC,   ,            AnyKey,  false)                \
  m(unsetElemI,   ,            IntKey,  false)                \
  m(unsetElemL,   ,            AnyKey,   true)                \
  m(unsetElemLI,  ,            IntKey,   true)                \
  m(unsetElemLS,  ,            StrKey,   true)                \
  m(unsetElemS,   ,            StrKey,  false)

#define ELEM(nm, hot, ...)                                      \
hot                                                             \
static void nm(TypedValue* base, TypedValue* key) {             \
  unsetElemImpl<__VA_ARGS__>(base, key);                        \
}
HELPER_TABLE(ELEM)
#undef ELEM

void TranslatorX64::emitUnsetElem(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii, unsigned mInd,
                                  unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  m_regMap.cleanSmashLoc(key.location);
  m_regMap.cleanSmashLoc(val.location);
  typedef void (*OpFunc)(TypedValue*, TypedValue*);
  BUILD_OPTABH(getKeyTypeIS(key), key.isRef());
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(key));
  assert(!ni.outLocal && !ni.outStack);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool isObj>
static inline void unsetPropImpl(Class* ctx, TypedValue* base,
                                 TypedValue* key) {
  key = unbox<keyType, unboxKey>(key);
  UnsetProp<isObj, keyType>(ctx, base, key);
}

#define HELPER_TABLE(m)                    \
  /* name           key unboxKey isObj */  \
  m(unsetPropC,   AnyKey, false, false)    \
  m(unsetPropCO,  AnyKey, false,  true)    \
  m(unsetPropL,   AnyKey,  true, false)    \
  m(unsetPropLO,  AnyKey,  true,  true)    \
  m(unsetPropLS,  StrKey,  true, false)    \
  m(unsetPropLSO, StrKey,  true,  true)    \
  m(unsetPropS,   StrKey, false, false)    \
  m(unsetPropSO,  StrKey, false,  true)

#define PROP(nm, ...)                                                   \
static void nm(Class* ctx, TypedValue* base, TypedValue* key) {         \
  unsetPropImpl<__VA_ARGS__>(ctx, base, key);                           \
}
HELPER_TABLE(PROP)
#undef PROP

void TranslatorX64::emitUnsetProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii,
                                  unsigned mInd, unsigned iInd,
                                  LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& key = *ni.inputs[iInd];
  const DynLocation& val = *ni.inputs[0];
  m_regMap.cleanSmashLoc(key.location);
  m_regMap.cleanSmashLoc(val.location);
  PREP_CTX(argNumToRegName[0]);
  // Emit the appropriate helper call.
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*);
  BUILD_OPTAB(getKeyTypeS(key), key.isRef(), m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(), R(rBase), SML(key));
  assert(!ni.outLocal && !ni.outStack);
}
#undef HELPER_TABLE

static inline void vGetNewElem(TypedValue* base, TypedValue* result,
                               MInstrState* mis) {
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
                                    unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  void (*vGetNewElemOp)(TypedValue*, TypedValue*, MInstrState*) = vGetNewElem;
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  EMIT_RCALL(a, ni, vGetNewElemOp,
                    R(rBase),
                    RESULT(useTvR),
                    R(mis_rsp));
}

static void setNewElemR(TypedValue* base, Cell* val) {
  SetNewElem<true>(base, val);
}

static void setNewElemNR(TypedValue* base, Cell* val) {
  SetNewElem<false>(base, val);
}

void TranslatorX64::emitSetNewElem(const Tracelet& t,
                                   const NormalizedInstruction& ni,
                                   const MInstrInfo& mii, unsigned mInd,
                                   unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  const DynLocation& val = *ni.inputs[0];
  m_regMap.cleanSmashLoc(val.location);
  // Emit the appropriate helper call.
  void (*setNewElemOp)(TypedValue*, Cell*) =
    setResult ? setNewElemR : setNewElemNR;
  bool useRVal = val.isRef();
  PREP_VAL(useRVal, argNumToRegName[1]);
  EMIT_RCALL(a, ni, setNewElemOp,
                    R(rBase),
                    VAL(useRVal));
}

template <unsigned char op, bool setResult>
static inline void setOpNewElemImpl(TypedValue* base, Cell* val,
                                    MInstrState* mis,
                                    TypedValue* tvRes=nullptr) {
  TypedValue* result = SetOpNewElem(mis->tvScratch, mis->tvRef, op, base, val);
  if (setResult) {
    if (result->m_type == KindOfRef) {
      tvUnbox(result);
    }
    tvDup(result, tvRes); // tvRes may or may not be &mis->tvResult.
  }
}

#define SETOP_OP(op, bcOp) \
static void setOp##op##NewElemR(TypedValue* base, Cell* val, \
                                MInstrState* mis, \
                                TypedValue* tvRes) { \
  setOpNewElemImpl<SetOp##op, true>(base, val, mis, tvRes); \
} \
static void setOp##op##NewElem(TypedValue* base, Cell* val, \
                               MInstrState* mis) { \
  setOpNewElemImpl<SetOp##op, false>(base, val, mis); \
}
SETOP_OPS
#undef SETOP_OP

void TranslatorX64::emitSetOpNewElem(const Tracelet& t,
                                     const NormalizedInstruction& ni,
                                     const MInstrInfo& mii, unsigned mInd,
                                     unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  unsigned char op = ni.imm[0].u_OA;
  const DynLocation& val = *ni.inputs[0];
  bool setResult = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
          __func__, setResult ? "true" : "false");
  m_regMap.cleanSmashLoc(val.location);
  bool useRVal = val.isRef();
  PREP_VAL(useRVal, argNumToRegName[3]);
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
                      R(mis_rsp),
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
                      R(mis_rsp));
  }
}

template <unsigned char op, bool setResult>
static inline void incDecNewElemImpl(TypedValue* base,
                                     MInstrState* mis,
                                     TypedValue* tvRes) {
  IncDecNewElem<setResult>(mis->tvScratch, mis->tvRef, op, base, *tvRes);
}

#define INCDEC_OP(op) \
static void incDec##op##NewElemR(TypedValue* base, \
                                 MInstrState* mis, \
                                 TypedValue* tvRes) { \
  incDecNewElemImpl<op, true>(base, mis, tvRes); \
} \
static void incDec##op##NewElem(TypedValue* base, \
                                MInstrState* mis) { \
  TypedValue tvRes; /* Not used; no need to initialize. */ \
  incDecNewElemImpl<op, false>(base, mis, &tvRes); \
}
INCDEC_OPS
#undef INCDEC_OP

void TranslatorX64::emitIncDecNewElem(const Tracelet& t,
                                      const NormalizedInstruction& ni,
                                      const MInstrInfo& mii, unsigned mInd,
                                      unsigned iInd, PhysReg rBase) {
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
                      R(mis_rsp),
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
    EMIT_RCALL(a, ni, incDecNewElemOp, R(rBase), R(mis_rsp));
  }
}

static inline void bindNewElem(TypedValue* base, TypedValue* val,
                               MInstrState* mis) {
  base = NewElem(mis->tvScratch, mis->tvRef, base);
  assert(val->m_type == KindOfRef);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val, base);
  }
}

void TranslatorX64::emitBindNewElem(const Tracelet& t,
                                    const NormalizedInstruction& ni,
                                    const MInstrInfo& mii, unsigned mInd,
                                    unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  assert(generateMVal(t, ni, mii));
  const DynLocation& val = *ni.inputs[0];
  m_regMap.cleanSmashLoc(val.location);
  assert(val.isRef());
  EMIT_RCALL(a, ni, bindNewElem, R(rBase), A(val.location), R(mis_rsp));
}

void TranslatorX64::emitNotSuppNewElem(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii, unsigned mInd,
                                       unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  not_reached();
}

void TranslatorX64::emitFinalMOp(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii,
                                 unsigned mInd,
                                 unsigned iInd,
                                 LazyScratchReg& rBase) {
  typedef void (TranslatorX64::*RegOp)(const Tracelet&,
                                       const NormalizedInstruction&,
                                       const MInstrInfo&, unsigned, unsigned,
                                       PhysReg rBase);
  typedef void (TranslatorX64::*ScratchOp)(const Tracelet&,
                                           const NormalizedInstruction&,
                                           const MInstrInfo&, unsigned,
                                           unsigned, LazyScratchReg& rBase);

  switch (ni.immVecM[mInd]) {
  case MEC: case MEL: case MET: case MEI:
    assert(!m_vecState->isKnown());
    static RegOp elemOps[] = {
#   define MII(instr, ...) &TranslatorX64::emit##instr##Elem,
    MINSTRS
#   undef MII
    };
    (this->*elemOps[mii.instr()])(t, ni, mii, mInd, iInd, r(rBase));
    break;
  case MPC: case MPL: case MPT:
    static ScratchOp propOps[] = {
#   define MII(instr, ...) &TranslatorX64::emit##instr##Prop,
    MINSTRS
#   undef MII
    };
    (this->*propOps[mii.instr()])(t, ni, mii, mInd, iInd, rBase);
    break;
  case MW:
    assert(!m_vecState->isKnown());
    assert(mii.getAttr(MW) & MIA_final);
    static RegOp newOp[] = {
#   define MII(instr, attrs, bS, iS, vC, fN) &TranslatorX64::emit##fN,
    MINSTRS
#   undef MII
    };
    (this->*newOp[mii.instr()])(t, ni, mii, mInd, iInd, r(rBase));
    break;
  default: not_reached();
  }
}

bool TranslatorX64::needMInstrCtx(const Tracelet& t,
                                  const NormalizedInstruction& ni) const {
  // Return true if the context will actually be used; otherwise return false
  // in order to avoid emission of getMInstrCtx() calls.
  switch (ni.immVec.locationCode()) {
  case LL: case LC: case LR: case LH:
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
                             const MInstrInfo& mii,
                             unsigned& mInd, unsigned& iInd,
                             LazyScratchReg& rBase) {
  if (!mInstrHasUnknownOffsets(ni, curFunc()->cls()) &&
      !useTvResult(t, ni, mii) &&
      (ni.mInstrOp() == OpCGetM || ni.mInstrOp() == OpSetM)) {
    m_vecState->setNoMIState();
  }
  SKTRACE(2, ni.source, "%s %#lx\n", __func__, long(a.code.frontier));
  if (m_vecState->needsMIState()) {
    if (debug) {
      emitStoreInvalid(a, MISOFF(tvScratch), mis_rsp);
    }
    SKTRACE(2, ni.source, "%s nLogicalRatchets=%u\n",
            __func__, nLogicalRatchets(t, ni, mii));
    if (nLogicalRatchets(t, ni, mii) > 0) {
      emitStoreUninitNull(a, MISOFF(tvRef), mis_rsp);
      emitStoreUninitNull(a, MISOFF(tvRef2), mis_rsp);
    } else if (debug) {
      emitStoreInvalid(a, MISOFF(tvRef), mis_rsp);
      emitStoreInvalid(a, MISOFF(tvRef2), mis_rsp);
    }
    if (useTvResult(t, ni, mii)) {
      emitStoreUninitNull(a, MISOFF(tvResult), mis_rsp);
    } else if (debug) {
      emitStoreInvalid(a, MISOFF(tvResult), mis_rsp);
    }
  }

  SKTRACE(2, ni.source, "%s\n", __func__);

  if (debug && m_vecState->needsMIState()) {
    a.  store_imm64_disp_reg64(0xfacefacefacefaceULL, MISOFF(ctx), mis_rsp);
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
    if (val.isRef()) {
      tmp.alloc();
      emitDerefRef(a, rVal, r(tmp));
      rVal = r(tmp);
    }
    // Copy val to tvVal for later assignment and decref.
    assert(val.valueType() == KindOfArray);
    emitStoreTypedValue(a, KindOfArray, rVal, MISOFF(tvVal), mis_rsp);
    // Incref, offset by decref in emitMPost().
    emitIncRef(rVal, KindOfArray);
  }

  // The base location is input 0 or 1, and the location code is stored
  // separately from ni.immVecM, so input indices (iInd) and member indices
  // (mInd) commonly differ.  Additionally, W members have no corresponding
  // inputs, so it is necessary to track the two indices separately.
  iInd = mii.valCount();
  emitBaseOp(t, ni, mii, iInd, rBase);
  ++iInd;
  // Iterate over all but the last member, which is consumed by a final
  // operation.
  for (mInd = 0; mInd < ni.immVecM.size() - 1; ++mInd) {
    emitIntermediateOp(t, ni, mii, mInd, iInd, rBase);
    emitRatchetRefs(t, ni, mii, mInd, r(rBase));
  }
}

void TranslatorX64::emitMPost(const Tracelet& t,
                              const NormalizedInstruction& ni,
                              const MInstrInfo& mii) {
  SKTRACE(2, ni.source, "%s %#lx\n", __func__, long(a.code.frontier));
  // Decref stack inputs.  Some instructions push their topmost input as their
  // final result; skip input 0 if it is a result.
  for (unsigned i = firstDecrefInput(t, ni, mii); i < ni.inputs.size(); ++i) {
    const DynLocation& input = *ni.inputs[i];
    switch (input.location.space) {
    case Location::Stack: {
      DataType dt = input.outerType();
      if (IS_REFCOUNTED_TYPE(dt)) {
        SKTRACE(2, ni.source, "%s %#lx decref stack input %u, type %s\n",
                __func__, long(a.code.frontier), i, tname(dt).c_str());
        PhysReg pr = m_regMap.allocReg(input.location, dt, RegInfo::CLEAN);
        emitDecRef(ni, pr, dt);
        // XXX: can't this go away since we're about to invalidate
        // popped stack locations after this call?
        m_regMap.cleanSmashLoc(input.location);
      }
      break;
    }
    case Location::Local:
    case Location::Litstr:
    case Location::Litint:
    case Location::This: {
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
    a.  load_reg64_disp_reg64(mis_rsp, MISOFF(tvVal) + TVOFF(m_data),
                              r(rScratch));
    emitDecRef(ni, r(rScratch), KindOfArray);
  }
  // Clean up ratchet.
  if (nLogicalRatchets(t, ni, mii) > 1) {
    SKTRACE(2, ni.source, "%s %#lx decref tvRef2\n",
            __func__, long(a.code.frontier));
    emitDecRefGeneric(ni, mis_rsp, MISOFF(tvRef2));
  }
  if (nLogicalRatchets(t, ni, mii) > 0) {
    SKTRACE(2, ni.source, "%s %#lx decref tvRef\n",
            __func__, long(a.code.frontier));
    emitDecRefGeneric(ni, mis_rsp, MISOFF(tvRef));
  }
  // Copy tvResult to final location if it was used.
  if (useTvResult(t, ni, mii)) {
    SKTRACE(2, ni.source, "%s %#lx copy tvResult\n",
            __func__, long(a.code.frontier));
    emitCopyToAligned(a, mis_rsp, MISOFF(tvResult),
                      rVmSp, vstackOffset(ni, mResultStackOffset(ni)));
    invalidateOutStack(ni);
  }
  // Teleport val to final location if this instruction generates val.
  if (teleportMVal(t, ni, mii)) {
    assert(!useTvResult(t, ni, mii));
    SKTRACE(2, ni.source, "%s %#lx teleport val\n",
            __func__, long(a.code.frontier));
    const DynLocation& val = *ni.inputs[0];
    m_regMap.cleanSmashLoc(val.location);
    PhysReg prVal;
    int dispVal;
    locToRegDisp(val.location, &prVal, &dispVal);
    emitCopyToAligned(a, prVal, dispVal,
                      rVmSp, vstackOffset(ni, mResultStackOffset(ni)));
    invalidateOutStack(ni);
  }
}

template <KeyType keyType, bool unboxKey>
static inline void cGetElemImpl(TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = Elem<true, keyType>(*result, mis->tvRef, base, key);
  if (base != result) {
    // Save a copy of the result.
    tvDup(base, result);
  }
  if (result->m_type == KindOfRef) {
    tvUnbox(result);
  }
}

#define HELPER_TABLE(m)                                          \
  /* name         hot         key   unboxKey */                  \
  m(cGetElemC,  ,            AnyKey,   false)                    \
  m(cGetElemI,  ,            IntKey,   false)                    \
  m(cGetElemL,  ,            AnyKey,    true)                    \
  m(cGetElemLI, ,            IntKey,    true)                    \
  m(cGetElemLS, ,            StrKey,    true)                    \
  m(cGetElemS,  ,            StrKey,   false)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
static void nm(TypedValue* base, TypedValue* key,                       \
                         TypedValue* result,                            \
                         MInstrState* mis) {             \
  cGetElemImpl<__VA_ARGS__>(base, key, result, mis);                    \
}
HELPER_TABLE(ELEM)
#undef ELEM

void TranslatorX64::emitCGetElem(const Tracelet& t,
                                 const NormalizedInstruction& ni,
                                 const MInstrInfo& mii, unsigned mInd,
                                 unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  const DynLocation& memb = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(memb.location);
  const DynLocation& result = *ni.outStack;
  bool useTvR = useTvResult(t, ni, mii);
  PREP_RESULT(useTvR);
  typedef void (*OpFunc)(TypedValue*, TypedValue*, TypedValue*, MInstrState*);
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isRef());
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(memb), RESULT(useTvR), R(mis_rsp));
  assert(!ni.outLocal);
  invalidateOutStack(ni);
}
#undef HELPER_TABLE

bool
isNormalPropertyAccess(const NormalizedInstruction& i,
                       int propInput,
                       int objInput) {
  const LocationCode lcode = i.immVec.locationCode();
  return
    i.immVecM.size() == 1 &&
    (lcode == LC || lcode == LL || lcode == LR || lcode == LH) &&
    mcodeMaybePropName(i.immVecM[0]) &&
    i.inputs[propInput]->isString() &&
    i.inputs[objInput]->valueType() == KindOfObject;
}

bool
mInstrHasUnknownOffsets(const NormalizedInstruction& ni, Class* context) {
  const MInstrInfo& mii = getMInstrInfo(ni.mInstrOp());
  unsigned mi = 0;
  unsigned ii = mii.valCount() + 1;
  for (; mi < ni.immVecM.size(); ++mi) {
    MemberCode mc = ni.immVecM[mi];
    if (mcodeMaybePropName(mc)) {
      const Class* cls = nullptr;
      if (getPropertyOffset(ni, context, cls, mii, mi, ii).offset == -1) {
        return true;
      }
      ++ii;
    } else {
      return true;
    }
  }

  return false;
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
isSupportedCGetM_RE(const NormalizedInstruction& i) {
  if (i.inputs.size() > 2) return false;
  return
    i.immVec.locationCode() == LR &&
    i.immVecM.size() == 1 &&
    mcodeMaybeArrayKey(i.immVecM[0]) &&
    i.inputs[0]->valueType() == KindOfArray && // base
    (i.inputs[1]->isInt() || i.inputs[1]->isString()); // key;
}

void
TranslatorX64::analyzeCGetM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = supportedPlan(isSupportedCGetM_LE(ni) ||
                                 isSupportedCGetM_RE(ni));
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
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
  PhysReg decRefReg = InvalidReg;
  bool stackSavedDecRef = false;
  if (decRefBase) {
    // We'll need to decref the base after the call. Make sure we hold
    // on to the value if it's a variant or from a global.
    if (baseInput->isRef()) {
      decRefReg = getReg(baseInput->location);
    }
    if (decRefReg != noreg && kCallerSaved.contains(decRefReg)) {
      stackSavedDecRef = true;
      a.    push(decRefReg);
      a.    sub_imm32_reg64(8, unsafe_rsp);
    }
  }

  int flags = 0;
  if (keyIn->isString()) {
    if (keyIn->isStack()) flags |= DecRefKey;
    if (!i.hasConstImm) flags |= CheckInts;
  }
  if (false) { // type-check
    ArrayData *a = nullptr;
    TypedValue tv;
    array_getm_i(a, 1, &tv);
    StringData *sd = nullptr;
    array_getm_s(a, sd, &tv, 0);
  }
  if (keyIn->isInt()) {
    EMIT_CALL(a, (void*)array_getm_i, R(baseReg), V(keyIn->location),
              A(outLoc));
  } else {
    EMIT_CALL(a, (void*)array_getm_s, R(baseReg), V(keyIn->location),
              A(outLoc), IMM(flags));
  }
  recordReentrantCall(i);
  m_regMap.invalidate(outLoc);

  if (decRefBase) {
    // For convenience of decRefs, the helpers return the ArrayData*.
    PhysReg base = rax;
    // but if it was boxed or from a global, we need to get the
    // original address back...
    if (decRefReg != noreg) {
      if (stackSavedDecRef) {
        a.    add_imm32_reg64(8, unsafe_rsp);
        a.    pop(rax);
      } else {
        base = decRefReg;
      }
    }
    emitDecRef(i, base, KindOfArray);
  }
}

void
TranslatorX64::translateCGetM(const Tracelet& t,
                              const NormalizedInstruction& i) {
  assert(i.inputs.size() >= 2);
  assert(i.outStack);

  if (isSupportedCGetM_LE(i) || isSupportedCGetM_RE(i)) {
    const DynLocation& base = *i.inputs[0];
    const DynLocation& key  = *i.inputs[1];
    int args[2];
    args[0] = base.isRef() ? ArgAnyReg : 0;
    args[1] = 1;
    allocInputsForCall(i, args);

    PhysReg baseReg = getReg(base.location);
    LazyScratchReg baseScratch(m_regMap);
    if (base.isRef()) {
      baseScratch.alloc();
      emitDerefRef(a, baseReg, r(baseScratch));
      baseReg = r(baseScratch);
    }
    Stats::emitInc(a, Stats::Tx64_CGetMArray);
    emitArrayElem(i, &base, baseReg, &key, i.outStack->location);
    return;
  }
  Stats::emitInc(a, Stats::Tx64_CGetMGeneric);
  translateMInstr(OpCGetM);
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
  translateMInstr(OpVGetM);
}

static bool isSupportedIssetMFast(const NormalizedInstruction& ni) {
  return ni.inputs.size() == 2 &&
    ni.immVec.locationCode() == LL &&
    ni.immVecM.size() == 1 &&
    mcodeMaybeArrayKey(ni.immVecM[0]) &&
    ni.inputs[0]->valueType() == KindOfArray &&
    (ni.inputs[1]->isString() || ni.inputs[1]->isInt());
}

void
TranslatorX64::analyzeIssetM(Tracelet& t, NormalizedInstruction& ni) {
  bool fast = isSupportedIssetMFast(ni);
  if (!RuntimeOption::EvalJitMGeneric) {
    ni.m_txFlags = supportedPlan(fast);
    return;
  }
  ni.m_txFlags = Supported;
  if (!fast) {
    assert(ni.isSupported());
    ni.manuallyAllocInputs = true;
  }
}

void TranslatorX64::translateIssetMFast(const Tracelet& t,
                                        const NormalizedInstruction& ni) {
  const DynLocation& base = *ni.inputs[0];
  const DynLocation& key  = *ni.inputs[1];

  PhysReg arrReg = getReg(base.location);

  LazyScratchReg scratch(m_regMap);
  if (base.isRef()) {
    scratch.alloc();
    emitDerefRef(a, arrReg, r(scratch));
    arrReg = r(scratch);
    SKTRACE(1, ni.source, "loaded variant\n");
  }

  typedef uint64_t (*HelperFunc)(const void* arr, StringData* sd);
  HelperFunc helper = nullptr;
  if (key.isInt()) {
    helper = (HelperFunc)array_issetm_i;
  } else {
    bool doIntCheck = !ni.hasConstImm;
    if (key.isStack()) {
      // on the stack, we need to decref strings.
      helper = doIntCheck ? array_issetm_s : array_issetm_s_fast;
    } else {
      assert(key.isLocal() || key.location.isLiteral());
      helper = doIntCheck ? array_issetm_s0 : array_issetm_s0_fast;
    }
  }
  assert(helper);
  // The array helpers can reenter; need to sync state.
  EMIT_RCALL(a, ni, helper, R(arrReg), V(key.location));

  // We didn't bother allocating the single output reg above;
  // it lives in rax now.
  assert(ni.outStack && !ni.outLocal);
  assert(ni.outStack->outerType() == KindOfBoolean);
  m_regMap.bind(rax, ni.outStack->location, KindOfBoolean,
                RegInfo::DIRTY);
}

void TranslatorX64::translateIssetM(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  if (isSupportedIssetMFast(ni)) {
    translateIssetMFast(t, ni);
  } else {
    assert(ni.isSupported());
    translateMInstr(OpIssetM);
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
  translateMInstr(OpEmptyM);
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

void
TranslatorX64::analyzeSetM(Tracelet& t, NormalizedInstruction& i) {
  // TODO: We could be more aggressive in the translation plans when the
  // lefthand side isn't refcounted.
  if (!RuntimeOption::EvalJitMGeneric) {
    i.m_txFlags = supportedPlan(isSupportedSetMArray(i));
    if (i.m_txFlags) {
      i.manuallyAllocInputs = true;
    }
    return;
  }
  i.m_txFlags = Supported;
  i.manuallyAllocInputs = true;
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
  assert(base.valueType() == KindOfArray || val.valueType() == KindOfArray);
  if (val.isLocal() &&
      (val.location == base.location ||
       (val.isRef() && base.isRef() &&
        val.valueType() == base.valueType()))) {
    // We don't allow a local input unless we also folded a following pop.
    assert(!ni.outStack);
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
    assert(ni.immVecM.size() > 1);
    if (val.isLocal()) {
      SKTRACE(2, ni.source, "%s true (val may alias one or more bases)\n",
              __func__, long(a.code.frontier));
      return true;
    }
    assert(val.isStack());
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
  assert(i.inputs.size() == 3);
  const DynLocation& val = *i.inputs[0];
  const DynLocation& arr = *i.inputs[1];
  const DynLocation& key = *i.inputs[2];
  assert(arr.isLocal());

  assert(val.isLocal() || !val.isRef());
  assert(!i.outStack || !val.isLocal());

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
  bool useBoxedForm = arr.isRef();
  void* fptr;
  if (false) { // helper type-checks
    RefData* ref = nullptr;
    ArrayData* arr = nullptr;
    TypedValue* rhs = nullptr;
    StringData* strKey = nullptr;
    UNUSED ArrayData* ret;
    ret = array_setm_ik1_v(ref, arr, 12, rhs);
    ret = array_setm_sk1_v(ref, arr, strKey, rhs);
    ret = array_setm_sk1_v0(ref, arr, strKey, rhs);
    ret = array_setm_s0k1_v(ref, arr, strKey, rhs);
    ret = array_setm_s0k1_v0(ref, arr, strKey, rhs);
    ret = array_setm_s0k1nc_v(ref, arr, strKey, rhs);
    ret = array_setm_s0k1nc_v0(ref, arr, strKey, rhs);
  }

  /*
   * The value can be passed as A or V according to:
   *
   *                       | val.isRef()     | !val.isRef()
   * ----------------------+-----------------+-----------------------------
   * value                 | V(valLoc)       | A(valLoc)
   */
  bool valIsRef = val.isRef();
  int args[3];
  args[0] = valIsRef ? 3 : ArgDontAllocate;
  args[1] = useBoxedForm ? 0 : 1;
  args[2] = 2;
  allocInputsForCall(i, args);

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

  if (forceIncDec) {
    LazyScratchReg tmp(m_regMap);
    PhysReg rhsReg = getReg(valLoc);
    if (valIsRef) {
      tmp.alloc();
      emitDerefRef(a, rhsReg, r(tmp));
      rhsReg = r(tmp);
    }
    emitIncRef(rhsReg, KindOfArray);
  }
  EMIT_CALL(a, fptr,
            useBoxedForm ? V(arrLoc) : IMM(0),
            useBoxedForm ? DEREF(arrLoc) : V(arrLoc),
            V(keyLoc),
            valIsRef ? VREF(valLoc) : A(valLoc));

  recordReentrantCall(i);
  // If we did not used boxed form, we need to tell the register allocator
  // to associate rax with arrLoc.
  if (!useBoxedForm) {
    // The array_setm helper returns the up-to-date array pointer in rax.
    // Therefore, we can bind rax to arrLoc and mark it as dirty.
    m_regMap.markAsClean(arrLoc);
    m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
  }

  if (i.outStack) {
    assert(!val.isRef());
    // Bring the output value into its correct location on the stack.
    // Note that this has to happen after all of the above, since it
    // needs to read the key from this location.
    m_regMap.allocInputReg(i, 0);
    m_regMap.bind(getReg(valLoc), i.outStack->location,
                  val.outerType(), RegInfo::DIRTY);
  }
}

void TranslatorX64::translateMInstr(Op op) {
  assert(RuntimeOption::EvalJitMGeneric);
  const MInstrInfo& mii = getMInstrInfo(op);
  const Tracelet& t = *m_curTrace;
  const NormalizedInstruction& ni = *m_curNI;

  SKTRACE(2, ni.source, "translate%sM\n", mii.name());
  m_vecState = new MVecTransState();
  Deleter<MVecTransState> stateDeleter(&m_vecState);
  unsigned mInd, iInd;
  LazyScratchReg rBase(m_regMap);
  emitMPre(t, ni, mii, mInd, iInd, rBase);
  emitFinalMOp(t, ni, mii, mInd, iInd, rBase);
  emitMPost(t, ni, mii);
}

void
TranslatorX64::translateSetM(const Tracelet& t,
                             const NormalizedInstruction& i) {
  if (isSupportedSetMArray(i)) {
    translateSetMArray(t, i);
    return;
  }
  translateMInstr(OpSetM);
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
  translateMInstr(OpSetOpM);
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
  translateMInstr(OpIncDecM);
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
  translateMInstr(OpUnsetM);
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
  translateMInstr(OpBindM);
}

void
TranslatorX64::analyzeFPassM(Tracelet& t, NormalizedInstruction& ni) {
  if (!RuntimeOption::EvalJitMGeneric) {
    if (!ni.preppedByRef) {
      analyzeCGetM(t, ni);
    } else {
      analyzeVGetM(t, ni);
    }
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
}

void
TranslatorX64::translateFPassM(const Tracelet& t,
                               const NormalizedInstruction& ni) {
  assert(ni.inputs.size() >= 1);
  assert(ni.outStack && !ni.outLocal);
  if (!ni.preppedByRef) {
    translateCGetM(t, ni);
  } else {
    translateVGetM(t, ni);
  }
}

}}
