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
#include <runtime/base/array/zend_array.h>
#include <runtime/base/shared/shared_map.h>

#include <runtime/vm/translator/translator-x64-internal.h>

#include <memory>

namespace HPHP {
namespace VM {
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
  ASSERT(condition);
  return value;
}
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
  IE(cond,                                                              \
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

#define PREP_CTX(pr)                                                   \
  Class* ctx = NULL;                                                   \
  LazyScratchReg rCtx(m_regMap);                                       \
  ctx = arGetContextClass(curFrame());
#define CTX()             \
  IE(true,                \
     IMM(uintptr_t(ctx)), \
     R(*rCtx))

#define PREP_RESULT(useTvR)                                                   \
  if (!useTvR) {                                                              \
    m_regMap.cleanSmashLoc(result.location);                                  \
    ScratchReg rScratch(m_regMap);                                            \
    a.  lea_reg64_disp_reg64(rVmSp, vstackOffset(ni, mResultStackOffset(ni)), \
                             *rScratch);                                      \
    emitStoreUninitNull(a, 0, *rScratch);                                     \
  }
#define RESULT(useTvR)                            \
  IE((useTvR),                                    \
     RPLUS(mis_rsp, MISOFF(tvResult)),            \
     A(result.location))
#define PREP_VAL(useRVal, pr)                                  \
  LazyScratchReg rVal(m_regMap);                               \
  if (useRVal) {                                               \
    rVal.alloc(pr);                                            \
    PhysReg r;                                                 \
    int disp;                                                  \
    locToRegDisp(val.location, &r, &disp);                     \
    a.  load_reg64_disp_reg64(r, disp + TVOFF(m_data), *rVal); \
  }
#define VAL(useRVal)  \
  IE((useRVal),       \
     R(*rVal),        \
     A(val.location))

#define TRANSLATE_MINSTR_GENERIC(instr, t, ni) do {                   \
  ASSERT(RuntimeOption::EvalJitMGeneric);                             \
  SKTRACE(2, ni.source, "%s\n", __func__);                            \
  m_vecState = new MVecTransState();                                  \
  Deleter<MVecTransState> stateDeleter(&m_vecState);                  \
  const MInstrInfo& mii = getMInstrInfo(Op##instr##M);                \
  unsigned mInd, iInd;                                                \
  LazyScratchReg rBase(m_regMap);                                     \
  emitMPre((t), (ni), mii, mInd, iInd, rBase);              \
  emitFinal##instr##MOp((t), (ni), mii, mInd, iInd, rBase); \
  emitMPost((t), (ni), mii);                                          \
} while (0)

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
    ASSERT(*dest == nullptr);                           \
    *dest = nm;                                         \
  } while (false);
#define FILL_ROWH(nm, hot, ...) FILL_ROW(nm, __VA_ARGS__)

#define BUILD_OPTAB(...) BUILD_OPTAB_ARG(HELPER_TABLE(FILL_ROW), __VA_ARGS__)
#define BUILD_OPTABH(...) BUILD_OPTAB_ARG(HELPER_TABLE(FILL_ROWH), __VA_ARGS__)
#define BUILD_OPTAB_ARG(FILL_TABLE, ...)                                \
  static OpFunc* optab = NULL;                                          \
  if (!optab) {                                                         \
    optab = (OpFunc*)calloc(1 << multiBitWidth(__VA_ARGS__), sizeof(OpFunc)); \
    FILL_TABLE                                                          \
  }                                                                     \
  unsigned idx = buildBitmask(__VA_ARGS__);                             \
  OpFunc opFunc = optab[idx];                                           \
  assert(opFunc);

// The getKeyType family of functions determine the KeyType to be used as a
// template argument to helper functions. S, IS, or I at the end of the
// function names signals that the caller supports non-literal strings, int, or
// both, respectively.
static KeyType getKeyType(const DynLocation& dl, bool nonLitStr,
                          bool nonLitInt) {
  if ((dl.isLiteral() || nonLitStr) && IS_STRING_TYPE(dl.valueType())) {
    return StrKey;
  } else if ((dl.isLiteral() || nonLitInt) && dl.valueType() == KindOfInt64) {
    return IntKey;
  } else {
    ASSERT(dl.isStack() || dl.isLocal());
    return AnyKey;
  }
}
inline static KeyType getKeyType(const DynLocation& dl) {
  return getKeyType(dl, false, false);
}
inline static KeyType getKeyTypeS(const DynLocation& dl) {
  return getKeyType(dl, true, false);
}
inline static KeyType getKeyTypeI(const DynLocation& dl) {
  return getKeyType(dl, false, true);
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
    if (input.isStack() && ni.outStack != NULL) {
      return true;
    }
  } else if (mii.instr() == MI_IncDecM) {
    if (ni.outStack != NULL) {
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

  int bottomI = 0;
  const DynLocation* bottomStack = NULL;
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
    ASSERT(bottomStack->location == output.location);
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
  bool result = mInstrHasUnknownOffsets(ni);
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
    if (base.isVariant()) {
      // Get inner value.
      a.  load_reg64_disp_reg64(pr, disp + TVOFF(m_data), *rBase);
    } else {
      a.  lea_reg64_disp_reg64(pr, disp, *rBase);
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
                                    TranslatorX64::MInstrState* mis,
                                    ActRec* fp) {
  TypedValue* base;
  StringData* name = prepareKey(key);
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

static TypedValue* baseN(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<false, false>(key, mis, (ActRec*)rbp->m_savedRbp);
}

static TypedValue* baseNW(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<true, false>(key, mis, (ActRec*)rbp->m_savedRbp);
}

static TypedValue* baseND(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<false, true>(key, mis, (ActRec*)rbp->m_savedRbp);
}

static TypedValue* baseNWD(TypedValue* key, TranslatorX64::MInstrState* mis) {
  register ActRec* rbp asm("rbp");
  return baseNImpl<true, true>(key, mis, (ActRec*)rbp->m_savedRbp);
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
  typedef TypedValue* (*BaseNOp)(TypedValue*, TranslatorX64::MInstrState*);
  static_assert(MIA_warn == 0x1 && MIA_define == 0x2,
                "MIA_* bitmask values were not as expected");
  static const BaseNOp baseNOps[] = {baseN, baseNW, baseND, baseNWD};
  ASSERT((mia & MIA_base) < sizeof(baseNOps)/sizeof(BaseNOp));
  BaseNOp baseNOp = baseNOps[mia & MIA_base];
  const DynLocation& base = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(base.location);
  EMIT_RCALL(a, ni, baseNOp, A(base.location), R(mis_rsp));
  rBase.alloc(rax);
}

template <bool warn, bool define>
static inline TypedValue* baseGImpl(TypedValue* key,
                                    TranslatorX64::MInstrState* mis) {
  TypedValue* base;
  StringData* name = prepareKey(key);
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

static TypedValue* baseG(TypedValue* key, TranslatorX64::MInstrState* mis) {
  return baseGImpl<false, false>(key, mis);
}

static TypedValue* baseGW(TypedValue* key, TranslatorX64::MInstrState* mis) {
  return baseGImpl<true, false>(key, mis);
}

static TypedValue* baseGD(TypedValue* key, TranslatorX64::MInstrState* mis) {
  return baseGImpl<false, true>(key, mis);
}

static TypedValue* baseGWD(TypedValue* key, TranslatorX64::MInstrState* mis) {
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
  typedef TypedValue* (*BaseGOp)(TypedValue*, TranslatorX64::MInstrState*);
  static_assert(MIA_warn == 0x1 && MIA_define == 0x2,
                "MIA_* bitmask values were not as expected");
  static const BaseGOp baseGOps[] = {baseG, baseGW, baseGD, baseGWD};
  ASSERT((mia & MIA_base) < sizeof(baseGOps)/sizeof(BaseGOp));
  BaseGOp baseGOp = baseGOps[mia & MIA_base];
  const DynLocation& base = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(base.location);
  EMIT_RCALL(a, ni, baseGOp, A(base.location), R(mis_rsp));
  rBase.alloc(rax);
}

HOT_FUNC_VM
static TypedValue* baseS(Class* ctx, TypedValue* key, const Class* cls,
                         TranslatorX64::MInstrState* mis) {
  TypedValue* base;
  StringData* name = prepareKey(key);
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
                              LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx\n", __func__, long(a.code.frontier));
  const DynLocation& key = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(key.location);
  const int kClassIdx = ni.inputs.size() - 1;
  const DynLocation& clsRef = *ni.inputs[kClassIdx];
  ASSERT(clsRef.valueType() == KindOfClass);
  const Class* cls = clsRef.rtt.valueClass();

  const bool uniqueKnownClass =
    cls != NULL &&
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
    emitStaticPropInlineLookup(*m_curNI, kClassIdx, key, *rBase);
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
      ASSERT(k->m_type == KindOfRef);
      k = k->m_data.pref->tv();
      ASSERT(k->m_type != KindOfRef);
    } else {
      ASSERT(k->m_type == keyDataType(kt) ||
             IS_STRING_TYPE(k->m_type) && IS_STRING_TYPE(keyDataType(kt)));
      return reinterpret_cast<TypedValue*>(k->m_data.num);
    }
  } else if (kt == AnyKey) {
    ASSERT(k->m_type != KindOfRef);
  }
  return k;
}

template<typename FuncType>
static inline FuncType helperFromKey(const DynLocation& keyDl,
                                     FuncType localHelper,
                                     FuncType cellHelper) {
  if (!keyDl.isVariant()) return cellHelper;
  return localHelper;
}

void TranslatorX64::emitHphpArrayGetIntKey(const NormalizedInstruction& i,
                                           PhysReg rBase,
                                           const DynLocation& keyLoc,
                                           Location outLoc,
                                           void* fallbackFunc) {
  // We're just going to use a heck-ton of scratch regs here. There is
  // a lot of state to carry around the loop.
  ScratchReg mask(m_regMap);
  ScratchReg hash(m_regMap);
  ScratchReg count(m_regMap);
  ScratchReg probe(m_regMap);
  LazyScratchReg rDereffedKey(m_regMap);
  PhysReg rKey = getReg(keyLoc.location);
  if (keyLoc.isVariant()) {
    rDereffedKey.alloc();
    emitDeref(a, rKey, *rDereffedKey);
    rKey = *rDereffedKey;
  }
  TCA bail = astubs.code.frontier;
  {
    // Failure path. A nasty subtelty is that we can't just use diamond
    // guard here; the helper is going to mutate the stack location that
    // backs rBase. The contract with the caller is that we're going to
    // preserve rBase bitwise. So, push and pop rBase around the diamond
    // guard.
    PhysRegSaver prs(astubs, RegSet(rBase));
    DiamondGuard dg(astubs);
    Stats::emitInc(astubs, Stats::ElemAsm_GetIMiss);
    EMIT_CALL(astubs, fallbackFunc,
              R(rBase), V(keyLoc.location), A(outLoc));
    recordReentrantStubCall(i);
  }
  TCA returnFromBail = astubs.code.frontier;
  astubs.jmp(astubs.code.frontier);
  {
    // Hold the register state steady; we might branch into bail.
    FreezeRegs brr(m_regMap);
    // Check that rBase is an HphpArray
    a.    cmp_imm64_disp_reg64(uintptr_t(HphpArray::getVTablePtr()),
                               0, rBase);
    a.    jnz(bail);
    a.    load_reg64_disp_reg32(rBase, HphpArray::getMaskOff(), *mask);
    a.    load_reg64_disp_reg64(rBase, HphpArray::getHashOff(), *hash);
    // The probe will be a uint32, so just start w/ low-order bits.
    a.    mov_reg32_reg32(rKey, *probe);
    emitImmReg(a, 0, *count);

    TCA loopHead = a.code.frontier; // loop:
    // probe = probe + count
    a.    lea_reg64_index_scale_disp_reg64(*count, *probe, 1, 0, *probe);
    a.    and_reg32_reg32(*mask, *probe);
    a.    load_reg64_index_scale_disp_reg32(*hash, *probe, 4, 0, *probe);
    // "probe" is now the index
    PhysReg index = *probe;
    a.    test_reg32_reg32(index, index);
    a.    js(bail);

    ASSERT(HphpArray::getElmSize() == 24);
    // index = index * 3; index = index * 8;
    a.    lea_reg64_index_scale_disp_reg64(index, index, 2, 0, index);
    a.    shl_imm32_reg32(3, index);

    a.    add_disp_reg64_reg64(HphpArray::getDataOff(), rBase, index);
    // index is now the elm pointer. Does the key look right?
    PhysReg elmPtr = index;
    a.    cmp_reg64_disp_reg64(rKey, HphpArray::getElmKeyOff(), elmPtr);
    TCA continue0 = a.code.frontier;
    a.    jcc8(CC_NZ, continue0); // Try again

    // Is it an int or a string?
    a.    cmp_imm32_disp_reg32(0, HphpArray::getElmHashOff(), elmPtr);
    TCA successJmp = a.code.frontier;
    a.    jcc8(CC_Z, successJmp);

    // Try the loop again.
    a.patchJcc8(continue0, a.code.frontier);
    a.    add_imm32_reg32(1, *count);
    a.    jmp8(loopHead);

    a.patchJcc8(successJmp, a.code.frontier);
    Stats::emitInc(a, Stats::ElemAsm_GetIHit);
    if (HphpArray::getElmDataOff() != 0) {
      a.  add_imm32_reg64(HphpArray::getElmDataOff(), elmPtr);
    }
    // Drat. Might be a Ref or a Indirect; for both, we should deref.
    a.    cmp_imm32_disp_reg32(KindOfRef, TVOFF(m_type), elmPtr);
    TCA skipDeref = a.code.frontier;
    a.    jl(skipDeref);
    emitDeref(a, elmPtr, elmPtr);
    a.patchJcc(skipDeref, a.code.frontier);
    // Copy to stack. We're done with mask, so reuse it as a scratch reg.
    emitCopyToStackRegSafe(a, i, elmPtr, mResultStackOffset(i), *mask);
    emitIncRefGenericRegSafe(elmPtr, 0, *mask);
  }
  astubs.patchJmp(returnFromBail, a.code.frontier);
}

template <KeyType keyType, bool unboxKey, bool warn, bool define, bool reffy,
          bool unset>
static inline TypedValue* elemImpl(TypedValue* base, TypedValue* key,
                                   TranslatorX64::MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  if (unset) {
    return ElemU<keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else if (define) {
    return ElemD<warn, reffy, keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else {
    return Elem<warn, keyType>(mis->tvScratch, mis->tvRef, base,
                               mis->baseStrOff, key);
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
  m(elemID,    HOT_FUNC_VM, IntKey,  false,   Define)                 \
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
  m(elemS,     HOT_FUNC_VM, StrKey,  false,   None)                   \
  m(elemSD,    HOT_FUNC_VM, StrKey,  false,   Define)                 \
  m(elemSDR,   ,            StrKey,  false,   DefineReffy)            \
  m(elemSU,    ,            StrKey,  false,   Unset)                  \
  m(elemSW,    HOT_FUNC_VM, StrKey,  false,   Warn)                   \
  m(elemSWD,   ,            StrKey,  false,   WarnDefine)             \
  m(elemSWDR,  ,            StrKey,  false,   WarnDefineReffy)

#define ELEM(nm, hot, keyType, unboxKey, attrs)                         \
hot                                                                     \
static TypedValue* nm(TypedValue* base, TypedValue* key,                \
                              TranslatorX64::MInstrState* mis) {        \
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
                                TranslatorX64::MInstrState*);
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isVariant(), mia);
  EMIT_RCALL(a, ni, opFunc, R(*rBase), ISML(memb), R(mis_rsp));
  rBase.realloc(rax);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, MInstrAttr attrs, bool isObj>
static inline TypedValue* propImpl(Class* ctx, TypedValue* base,
                                   TypedValue* key,
                                   TranslatorX64::MInstrState* mis) {
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
                              TranslatorX64::MInstrState* mis) {        \
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
                                TranslatorX64::MInstrState*);
  BUILD_OPTAB(getKeyTypeS(memb), memb.isVariant(), mia, m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(),
                    R(*rBase),
                    SML(memb),
                    R(mis_rsp));
  rBase.realloc(rax);
  m_vecState->resetBase();
}
#undef HELPER_TABLE

static int getPropertyOffset(const NormalizedInstruction& ni,
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
  if (!baseClass) return -1;

  if (!ni.inputs[iInd]->rtt.isString()) {
    return -1;
  }
  auto* const name = ni.inputs[iInd]->rtt.valueString();
  if (!name) return -1;

  bool accessible;
  Class* ctx = curFunc()->cls();
  // If we are not in repo-authoriative mode, we need to check that
  // baseClass cannot change in between requests
  if (!RuntimeOption::RepoAuthoritative ||
      !(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return -1;
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return -1;
      }
    }
  }
  // Lookup the index of the property based on ctx and baseClass
  Slot idx = baseClass->getDeclPropIndex(ctx, name, accessible);
  // If we couldn't find a property that is accessible in the current
  // context, bail out
  if (idx == kInvalidSlot || !accessible) {
    return -1;
  }
  // If it's a declared property we're good to go: even if a subclass
  // redefines an accessible property with the same name it's guaranteed
  // to be at the same offset
  return baseClass->declPropOffset(idx);
}

static void raiseUndefProp(ObjectData* base, const StringData* name) {
  static_cast<Instance*>(base)->raiseUndefProp(name);
}

static void raisePropertyOnNonObject() {
  raise_warning("Cannot access property on non-object");
}

void TranslatorX64::emitPropSpecialized(MInstrAttr const mia,
                                        const Class* baseClass,
                                        int propOffset,
                                        unsigned mInd,
                                        unsigned iInd,
                                        LazyScratchReg& rBase) {
  SKTRACE(2, m_curNI->source, "%s class=%s offset=%d\n",
    __func__, baseClass->nameRef()->data(), propOffset);

  ASSERT(!(mia & MIA_warn) || !(mia & MIA_unset));
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
    emitTypeCheck(a, KindOfObject, *rBase, 0);
    {
      nonObjectRet.reset(new DiamondReturn());
      UnlikelyIfBlock<CC_NZ> ifNotObject(a, astubs, nonObjectRet.get());
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
        emitImmReg(astubs, uintptr_t(&init_null_variant), *rBase);
      }
    }
  }

  ScratchReg rScratch(m_regMap);

  if (!isObj) {
    emitDeref(a, *rBase, *rBase);
  }
  a.    lea_reg64_disp_reg64(*rBase, propOffset, *rScratch);
  if (doWarn || doDefine) {
    a.  cmp_imm32_disp_reg32(KindOfUninit, TVOFF(m_type), *rScratch);
    {
      UnlikelyIfBlock<CC_Z> ifUninit(a, astubs);
      if (doWarn) {
        EMIT_RCALL(
          astubs, *m_curNI, raiseUndefProp,
          R(*rBase),
          IMM(uintptr_t(m_curNI->inputs[iInd]->rtt.valueString()))
        );
      }
      if (doDefine) {
        astubs.store_imm32_disp_reg(KindOfNull, TVOFF(m_type), *rScratch);
      } else {
        emitImmReg(astubs, uintptr_t(&init_null_variant), *rScratch);
      }
    }
  }

  // We have to prevent ~ScratchReg from deallocating this Scratch, so
  // unbind it before doing the realloc.
  auto usedScratch = *rScratch;
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
  const int propOffset  = getPropertyOffset(*m_curNI, knownCls, mii,
                                            mInd, iInd);

  if (propOffset == -1) {
    emitPropGeneric(*m_curTrace, *m_curNI, mii, mInd, iInd, rBase);
  } else {
    auto attrs = mii.getAttr(m_curNI->immVecM[mInd]);
    emitPropSpecialized(attrs, knownCls, propOffset, mInd, iInd, rBase);
  }
}

static TypedValue* newElem(TypedValue* base, TranslatorX64::MInstrState* mis) {
  return NewElem(mis->tvScratch, mis->tvRef, base);
}

void TranslatorX64::emitNewElem(const Tracelet& t,
                                const NormalizedInstruction& ni,
                                unsigned mInd, LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u\n",
          __func__, long(a.code.frontier), mInd);
  EMIT_RCALL(a, ni, newElem, R(*rBase), R(mis_rsp));
  rBase.realloc(rax);
}

void TranslatorX64::emitIntermediateOp(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii,
                                       unsigned mInd, unsigned& iInd,
                                       LazyScratchReg& rBase) {
  switch (ni.immVecM[mInd]) {
  case MEC: case MEL: case MET: case MEI: {
    ASSERT(!m_vecState->isKnown());
    emitElem(t, ni, mii, mInd, iInd, rBase);
    ++iInd;
    break;
  }
  case MPC: case MPL: case MPT:
    emitProp(mii, mInd, iInd, rBase);
    ++iInd;
    break;
  case MW:
    ASSERT(!m_vecState->isKnown());
    ASSERT(mii.newElem());
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
      ASSERT(sizeof(TypedValue) % 8 == 0);
      for (size_t off = 0; off < sizeof(TypedValue); off += 8) {
        a.load_reg64_disp_reg64(mis_rsp, MISOFF(tvRef) + off, *rScratch);
        a.store_reg64_disp_reg64(*rScratch, MISOFF(tvRef2) + off, mis_rsp);
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
                                TranslatorX64::MInstrState* mis) {
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
  m(cGetPropSO,   HOT_FUNC_VM, StrKey, false,  true)

#define PROP(nm, hot, ...)                                              \
hot                                                                     \
static void nm(Class* ctx, TypedValue* base, TypedValue* key,           \
                           TypedValue* result,                          \
                           TranslatorX64::MInstrState* mis) {           \
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
  ASSERT(!ni.outLocal);

  /*
   * If we know the class for the current base, emit a direct property
   * access.
   */
  const Class* knownCls = nullptr;
  const int propOffset  = getPropertyOffset(*m_curNI, knownCls,
                                            mii, mInd, iInd);
  if (propOffset != -1) {
    emitPropSpecialized(MIA_warn, knownCls, propOffset, mInd, iInd, rBase);
    emitDerefIfVariant(a, *rBase);
    emitIncRefGeneric(*rBase, 0);

    PhysReg stackOutReg;
    int stackOutDisp;
    if (useTvResult(t, ni, mii)) {
      stackOutReg = mis_rsp;
      stackOutDisp = MISOFF(tvResult);
    } else {
      locToRegDisp(ni.outStack->location, &stackOutReg, &stackOutDisp);
      invalidateOutStack(ni);
    }

    ScratchReg scratch(m_regMap);
    emitCopyTo(a, *rBase, 0, stackOutReg, stackOutDisp, *scratch);
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
  BUILD_OPTABH(getKeyTypeS(memb), memb.isVariant(), m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc,
             CTX(), R(*rBase), SML(memb), RESULT(useTvR), R(mis_rsp));
  if (!useTvR) invalidateOutStack(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey>
static inline void vGetElemImpl(TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                TranslatorX64::MInstrState* mis) {
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
  m(vGetElemI,   HOT_FUNC_VM, IntKey,   false)                    \
  m(vGetElemL,   ,            AnyKey,    true)                    \
  m(vGetElemLI,  ,            IntKey,    true)                    \
  m(vGetElemLS,  ,            StrKey,    true)                    \
  m(vGetElemS,   ,            StrKey,   false)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
static void nm(TypedValue* base, TypedValue* key, TypedValue* result,   \
               TranslatorX64::MInstrState* mis) {                       \
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
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isVariant());
  cleanOutLocal(ni);
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(memb), RESULT(useTvR), R(mis_rsp));
  invalidateOutLocal(ni);
  if (!useTvR) invalidateOutStack(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool isObj>
static inline void vGetPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                TranslatorX64::MInstrState* mis) {
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
  m(vGetPropSO,  HOT_FUNC_VM, StrKey, false,  true)

#define PROP(nm, hot, ...)                                              \
hot                                                                     \
static void nm(Class* ctx, TypedValue* base, TypedValue* key,           \
               TypedValue* result,                                      \
               TranslatorX64::MInstrState* mis) {                       \
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
  BUILD_OPTABH(getKeyTypeS(memb), memb.isVariant(), m_vecState->isObj());
  cleanOutLocal(ni);
  EMIT_RCALL(a, ni, opFunc,
             CTX(), R(*rBase), SML(memb), RESULT(useTvR), R(mis_rsp));
  invalidateOutLocal(ni);
  if (!useTvR) invalidateOutStack(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool useEmpty>
static inline bool issetEmptyElemImpl(TypedValue* base, TypedValue* key,
                                      TranslatorX64::MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  return IssetEmptyElem<useEmpty, false, keyType>(mis->tvScratch, mis->tvRef,
                                                  base, mis->baseStrOff, key);
}

#define HELPER_TABLE(m)                                      \
  /* name          hot        keyType unboxKey useEmpty */   \
  m(issetElemC,   ,            AnyKey, false,  false)        \
  m(issetElemCE,  ,            AnyKey, false,   true)        \
  m(issetElemI,   HOT_FUNC_VM, IntKey, false,  false)        \
  m(issetElemIE,  ,            IntKey, false,   true)        \
  m(issetElemL,   ,            AnyKey,  true,  false)        \
  m(issetElemLE,  ,            AnyKey,  true,   true)        \
  m(issetElemLI,  ,            IntKey,  true,  false)        \
  m(issetElemLIE, ,            IntKey,  true,   true)        \
  m(issetElemLS,  ,            StrKey,  true,  false)        \
  m(issetElemLSE, ,            StrKey,  true,   true)        \
  m(issetElemS,   HOT_FUNC_VM, StrKey, false,  false)        \
  m(issetElemSE,  HOT_FUNC_VM, StrKey, false,   true)

#define ISSET(nm, hot, ...)                                     \
hot                                                             \
static bool nm(TypedValue* base, TypedValue* key,               \
               TranslatorX64::MInstrState* mis) {               \
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
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isVariant(), useEmpty);
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(memb), R(mis_rsp));
  a.   and_imm32_reg64(1, rax); // Mask garbage bits.
  ScratchReg rIssetEmpty(m_regMap, rax);
  ASSERT(!ni.outLocal);
  if (useTvResult(t, ni, mii)) {
    emitStoreTypedValue(a, KindOfBoolean, *rIssetEmpty, MISOFF(tvResult),
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
                                      TranslatorX64::MInstrState* mis) {
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
  m(issetPropSO,   HOT_FUNC_VM, StrKey, false, false,    true)

#define ISSET(nm, hot, ...)                                             \
hot                                                                     \
static bool nm(Class* ctx, TypedValue* base, TypedValue* key,           \
               TranslatorX64::MInstrState* mis) {                       \
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
  BUILD_OPTABH(getKeyTypeS(memb), memb.isVariant(), useEmpty,
              m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(), R(rBase), SML(memb));
  a.   and_imm32_reg64(1, rax); // Mask garbage bits.
  ScratchReg rIssetEmpty(m_regMap, rax);
  ASSERT(!ni.outLocal);
  if (useTvResult(t, ni, mii)) {
    emitStoreTypedValue(a, KindOfBoolean, *rIssetEmpty, MISOFF(tvResult),
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
  emitIssetEmptyProp<false>(t, ni, mii, mInd, iInd, *rBase);
}

void TranslatorX64::emitEmptyProp(const Tracelet& t,
                                  const NormalizedInstruction& ni,
                                  const MInstrInfo& mii,
                                  unsigned mInd, unsigned iInd,
                                  LazyScratchReg& rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  emitIssetEmptyProp<true>(t, ni, mii, mInd, iInd, *rBase);
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
  m(setElemS,   HOT_FUNC_VM, StrKey, false, false)             \
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
  bool useRVal = (!forceMValIncDec(t, ni, mii) && val.isVariant());
  PREP_VAL(useRVal, argNumToRegName[2]);
  // Emit the appropriate helper call.
  typedef void (*OpFunc)(TypedValue*, TypedValue*, Cell*);
  BUILD_OPTABH(getKeyTypeIS(key), key.isVariant(), setResult);
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
  const int propOffset  = getPropertyOffset(*m_curNI, knownCls,
                                            mii, mInd, iInd);
  if (propOffset != -1 && !ni.outLocal && !ni.outStack) {
    emitPropSpecialized(MIA_define, knownCls, propOffset, mInd, iInd, rBase);

    m_regMap.allocInputReg(*m_curNI, kRhsIdx);
    PhysReg rhsReg = getReg(val.location);
    LazyScratchReg tmp(m_regMap);
    if (val.isVariant() && !IS_NULL_TYPE(val.rtt.valueType())) {
      tmp.alloc();
      emitDeref(a, rhsReg, *tmp);
      rhsReg = *tmp;
    }

    const bool incRef = true;
    emitTvSet(*m_curNI, rhsReg, val.rtt.valueType(), *rBase, 0, incRef);
    return;
  }

  Stats::emitInc(a, Stats::PropAsm_GenFinal);

  const bool setResult   = generateMVal(t, ni, mii);
  SKTRACE(2, ni.source, "%s setResult=%s\n",
        __func__, setResult ? "true" : "false");
  const DynLocation& key = *ni.inputs[iInd];
  m_regMap.cleanSmashLoc(val.location);
  PREP_CTX(argNumToRegName[0]);
  bool useRVal = val.isVariant();
  PREP_VAL(useRVal, argNumToRegName[3]);
  // Emit the appropriate helper call.
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, Cell*);
  cleanOutLocal(ni);
  BUILD_OPTAB(getKeyTypeS(key), key.isVariant(), setResult,
              m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(), R(*rBase), SML(key), VAL(useRVal));
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, SetOpOp op, bool setResult>
static inline void setOpElemImpl(TypedValue* base, TypedValue* key, Cell* val,
                                 TranslatorX64::MInstrState* mis,
                                 TypedValue* tvRes=NULL) {
  key = unbox<keyType, unboxKey>(key);
  TypedValue* result = SetOpElem<keyType>(mis->tvScratch, mis->tvRef, op, base,
                                          key, val);
  if (setResult) {
    if (result->m_type == KindOfRef) {
      tvUnbox(result);
    }
    tvDup(result, tvRes); // tvRes may or may not be &mis->tvResult.
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
               TranslatorX64::MInstrState* mis,                         \
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
  bool useRVal = (!forceMValIncDec(t, ni, mii) && val.isVariant());
  PREP_VAL(useRVal, argNumToRegName[2]);
  typedef void (*OpFunc)(TypedValue*, TypedValue*, Cell*, MInstrState*,
                         TypedValue*);
# define SETOP_OP(op, bcOp) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(SETOP_OPS, getKeyTypeIS(key), key.isVariant(), op, setResult);
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
                                 Cell* val, TranslatorX64::MInstrState* mis,
                                 TypedValue* tvRes=NULL) {
  key = unbox<keyType, unboxKey>(key);
  TypedValue* result = SetOpProp<isObj, keyType>(mis->tvScratch, mis->tvRef,
                                                 ctx, op, base, key, val);
  if (setResult) {
    if (result->m_type == KindOfRef) {
      tvUnbox(result);
    }
    tvDup(result, tvRes); // tvRes may or may not be &mis->tvResult.
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
               Cell* val, TranslatorX64::MInstrState* mis,              \
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
  bool useRVal = val.isVariant();
  bool isObj = m_vecState->isObj();
  PREP_VAL(useRVal, argNumToRegName[3]);
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, Cell*, MInstrState*,
                         TypedValue*);
# define SETOP_OP(op, bcOp) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(SETOP_OPS,
                  getKeyTypeS(key), key.isVariant(), op, setResult, isObj);
# undef SETOP_OP
  // Emit the appropriate helper call.
  cleanOutLocal(ni);
  if (setResult) {
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, opFunc,
                      CTX(),
                      R(*rBase),
                      SML(key),
                      VAL(useRVal),
                      R(mis_rsp),
                      RESULT(useTvR));
  if (!useTvR) invalidateOutStack(ni);
  } else {
    EMIT_RCALL(a, ni, opFunc,
               CTX(), R(*rBase), SML(key), VAL(useRVal), R(mis_rsp));
  }
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, IncDecOp op, bool setResult>
static inline void incDecElemImpl(TypedValue* base, TypedValue* key,
                                  TranslatorX64::MInstrState* mis,
                                  TypedValue* tvRes) {
  key = unbox<keyType, unboxKey>(key);
  IncDecElem<setResult, keyType>(mis->tvScratch, mis->tvRef, op, base, key,
                                 *tvRes);
}

#define HELPER_TABLE(m, op) OPELEM_TABLE(m, incDec, op)
#define INCDEC(nm, ...)                                           \
static void nm(TypedValue* base, TypedValue* key,                 \
               TranslatorX64::MInstrState* mis,                   \
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
  BUILD_OPTAB_ARG(INCDEC_OPS, getKeyTypeIS(key), key.isVariant(), op, setResult);
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
                                  TranslatorX64::MInstrState* mis,
                                  TypedValue* tvRes) {
  key = unbox<keyType, unboxKey>(key);
  IncDecProp<setResult, isObj, keyType>(mis->tvScratch, mis->tvRef, ctx, op,
                                        base, key, *tvRes);
}


#define HELPER_TABLE(m, op) OPPROP_TABLE(m, incDec, op)
#define INCDEC(nm, ...)                                                 \
static void nm(Class* ctx, TypedValue* base, TypedValue* key,           \
               TranslatorX64::MInstrState* mis, TypedValue* tvRes) {    \
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
                  getKeyTypeS(key), key.isVariant(), op, setResult, isObj);
# undef INCDEC_OP
  // Emit the appropriate helper call.
  cleanOutLocal(ni);
  if (setResult) {
    const DynLocation& result = *ni.outStack;
    bool useTvR = useTvResult(t, ni, mii);
    PREP_RESULT(useTvR);
    EMIT_RCALL(a, ni, opFunc,
                      CTX(),
                      R(*rBase),
                      SML(key),
                      R(mis_rsp),
                      RESULT(useTvR));
    if (!useTvR) invalidateOutStack(ni);
  } else {
    EMIT_RCALL(a, ni, opFunc,
               CTX(), R(*rBase), SML(key), R(mis_rsp));
  }
  invalidateOutLocal(ni);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey>
static inline void bindElemImpl(TypedValue* base, TypedValue* key, RefData* val,
                                TranslatorX64::MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = ElemD<false, true, keyType>(mis->tvScratch, mis->tvRef, base, key);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val->tv(), base);
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
static void nm(TypedValue* base, TypedValue* key, RefData* val,         \
               TranslatorX64::MInstrState* mis) {                       \
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
  ASSERT(generateMVal(t, ni, mii));
  m_regMap.cleanSmashLoc(val.location);
  ASSERT(!forceMValIncDec(t, ni, mii));
  ASSERT(val.isVariant());
  typedef void (*OpFunc)(TypedValue*, TypedValue*, RefData*, MInstrState*);
  BUILD_OPTAB(getKeyTypeIS(key), key.isVariant());
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(key), A(val.location), R(mis_rsp));
  ASSERT(!ni.outLocal);
}
#undef HELPER_TABLE

template <KeyType keyType, bool unboxKey, bool isObj>
static inline void bindPropImpl(Class* ctx, TypedValue* base, TypedValue* key,
                                RefData* val, TranslatorX64::MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = Prop<false, true, false, isObj, keyType>(mis->tvScratch, mis->tvRef,
                                                  ctx, base, key);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBind(val->tv(), base);
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
                                RefData* val,                           \
                                TranslatorX64::MInstrState* mis) {      \
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
  ASSERT(val.isVariant());
  ASSERT(generateMVal(t, ni, mii));
  const DynLocation& key = *ni.inputs[iInd];
  PREP_CTX(argNumToRegName[0]);
  // Emit the appropriate helper call.
  ASSERT(!forceMValIncDec(t, ni, mii));
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, RefData*,
                         MInstrState*);
  BUILD_OPTAB(getKeyTypeS(key), key.isVariant(), m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc,
             CTX(), R(*rBase), SML(key), A(val.location), R(mis_rsp));
  ASSERT(!ni.outLocal);
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
  m(unsetElemI,   HOT_FUNC_VM, IntKey,  false)                \
  m(unsetElemL,   ,            AnyKey,   true)                \
  m(unsetElemLI,  ,            IntKey,   true)                \
  m(unsetElemLS,  ,            StrKey,   true)                \
  m(unsetElemS,   HOT_FUNC_VM, StrKey,  false)

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
  BUILD_OPTABH(getKeyTypeIS(key), key.isVariant());
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(key));
  ASSERT(!ni.outLocal && !ni.outStack);
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
  BUILD_OPTAB(getKeyTypeS(key), key.isVariant(), m_vecState->isObj());
  EMIT_RCALL(a, ni, opFunc, CTX(), R(*rBase), SML(key));
  ASSERT(!ni.outLocal && !ni.outStack);
}
#undef HELPER_TABLE

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

HOT_FUNC_VM
static void setNewElem(TypedValue* base, Cell* val) {
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
    setResult ? setNewElemR : setNewElem;
  bool useRVal = val.isVariant();
  PREP_VAL(useRVal, argNumToRegName[1]);
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
static void setOp##op##NewElemR(TypedValue* base, Cell* val, \
                                TranslatorX64::MInstrState* mis, \
                                TypedValue* tvRes) { \
  setOpNewElemImpl<SetOp##op, true>(base, val, mis, tvRes); \
} \
static void setOp##op##NewElem(TypedValue* base, Cell* val, \
                               TranslatorX64::MInstrState* mis) { \
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
  bool useRVal = val.isVariant();
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
                                     TranslatorX64::MInstrState* mis,
                                     TypedValue* tvRes) {
  IncDecNewElem<setResult>(mis->tvScratch, mis->tvRef, op, base, *tvRes);
}

#define INCDEC_OP(op) \
static void incDec##op##NewElemR(TypedValue* base, \
                                 TranslatorX64::MInstrState* mis, \
                                 TypedValue* tvRes) { \
  incDecNewElemImpl<op, true>(base, mis, tvRes); \
} \
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
                                    unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  ASSERT(generateMVal(t, ni, mii));
  const DynLocation& val = *ni.inputs[0];
  m_regMap.cleanSmashLoc(val.location);
  // Emit the appropriate helper call.
  void (*bindNewElemOp)(TypedValue*, RefData*, MInstrState*) = bindNewElem;
  ASSERT(val.isVariant());
  EMIT_RCALL(a, ni, bindNewElemOp, R(rBase), A(val.location), R(mis_rsp));
}

void TranslatorX64::emitNotSuppNewElem(const Tracelet& t,
                                       const NormalizedInstruction& ni,
                                       const MInstrInfo& mii, unsigned mInd,
                                       unsigned iInd, PhysReg rBase) {
  SKTRACE(2, ni.source, "%s %#lx mInd=%u, iInd=%u\n",
          __func__, long(a.code.frontier), mInd, iInd);
  not_reached();
}

#define MII(instr, attrs, bS, iS, vC, fN) \
void TranslatorX64::emitFinal##instr##MOp(const Tracelet& t, \
                                          const NormalizedInstruction& ni, \
                                          const MInstrInfo& mii, \
                                          unsigned mInd, \
                                          unsigned iInd, \
                                          LazyScratchReg& rBase) { \
  switch (ni.immVecM[mInd]) { \
  case MEC: case MEL: case MET: case MEI: \
    ASSERT(!m_vecState->isKnown()); \
    emit##instr##Elem(t, ni, mii, mInd, iInd, *rBase); \
    break; \
  case MPC: case MPL: case MPT: \
    emit##instr##Prop(t, ni, mii, mInd, iInd, rBase); \
    break; \
  case MW: \
    ASSERT(!m_vecState->isKnown()); \
    ASSERT((attrs) & MIA_final); \
    emit##fN(t, ni, mii, mInd, iInd, *rBase); \
    break; \
  default: not_reached(); \
  } \
}
MINSTRS
#undef MII

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
  if (!mInstrHasUnknownOffsets(ni) && !useTvResult(t, ni, mii) &&
      ni.mInstrOp() == OpCGetM) {
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
    a.  store_imm32_disp_reg(false, MISOFF(baseStrOff), mis_rsp);
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
    if (val.isVariant()) {
      tmp.alloc();
      emitDeref(a, rVal, *tmp);
      rVal = *tmp;
    }
    // Copy val to tvVal for later assignment and decref.
    ASSERT(val.valueType() == KindOfArray);
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
    emitRatchetRefs(t, ni, mii, mInd, *rBase);
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
                              *rScratch);
    emitDecRef(ni, *rScratch, KindOfArray);
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
    ScratchReg rTvResult(m_regMap);
    a.  lea_reg64_disp_reg64(mis_rsp, MISOFF(tvResult), *rTvResult);
    ScratchReg rScratch(m_regMap);
    emitCopyToStackRegSafe(a, ni, *rTvResult, mResultStackOffset(ni),
                           *rScratch);
    invalidateOutStack(ni);
  }
  // Teleport val to final location if this instruction generates val.
  if (teleportMVal(t, ni, mii)) {
    ASSERT(!useTvResult(t, ni, mii));
    SKTRACE(2, ni.source, "%s %#lx teleport val\n",
            __func__, long(a.code.frontier));
    ScratchReg rScratch(m_regMap);
    const DynLocation& val = *ni.inputs[0];
    m_regMap.cleanSmashLoc(val.location);
    PhysReg prVal;
    int dispVal;
    locToRegDisp(val.location, &prVal, &dispVal);
    emitCopyTo(a, prVal, dispVal, rVmSp,
               vstackOffset(ni, mResultStackOffset(ni)), *rScratch);
    invalidateOutStack(ni);
  }
}

template <KeyType keyType, bool unboxKey>
static inline void cGetElemImpl(TypedValue* base, TypedValue* key,
                                TypedValue* result,
                                TranslatorX64::MInstrState* mis) {
  key = unbox<keyType, unboxKey>(key);
  base = Elem<true, keyType>(*result, mis->tvRef, base, mis->baseStrOff, key);
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
  m(cGetElemL,  HOT_FUNC_VM, AnyKey,    true)                    \
  m(cGetElemLI, HOT_FUNC_VM, IntKey,    true)                    \
  m(cGetElemLS, ,            StrKey,    true)                    \
  m(cGetElemS,  HOT_FUNC_VM, StrKey,   false)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
static void nm(TypedValue* base, TypedValue* key,                       \
                         TypedValue* result,                            \
                         TranslatorX64::MInstrState* mis) {             \
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
  BUILD_OPTABH(getKeyTypeIS(memb), memb.isVariant());
  EMIT_RCALL(a, ni, opFunc, R(rBase), ISML(memb), RESULT(useTvR), R(mis_rsp));
  ASSERT(!ni.outLocal);
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

int getNormalPropertyOffset(const NormalizedInstruction& i,
                            const MInstrInfo& mii,
                            int propInput, int objInput) {
  ASSERT(isNormalPropertyAccess(i, propInput, objInput));
  const Class* baseClass = nullptr;
  return getPropertyOffset(i, baseClass, mii, objInput, propInput);
}

bool
mInstrHasUnknownOffsets(const NormalizedInstruction& ni) {
  const MInstrInfo& mii = getMInstrInfo(ni.mInstrOp());
  unsigned mi = 0;
  unsigned ii = mii.valCount() + 1;
  for (; mi < ni.immVecM.size(); ++mi) {
    MemberCode mc = ni.immVecM[mi];
    if (mcodeMaybePropName(mc)) {
      const Class* cls = NULL;
      if (getPropertyOffset(ni, cls, mii, mi, ii) == -1) {
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
    ni.m_txFlags = supportedPlan(isSupportedCGetM_LEE(ni) ||
                                 isSupportedCGetM_GE(ni) ||
                                 isSupportedCGetM_LE(ni) ||
                                 isSupportedCGetM_RE(ni));
    return;
  }
  ni.m_txFlags = Supported;
  ni.manuallyAllocInputs = true;
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

  if (base.isStack()) {
    // Release the base
    emitDecRef(i, baseReg, base.outerType());
  }
}

void
TranslatorX64::emitArrayElem(const NormalizedInstruction& i,
                             const DynLocation* baseInput,
                             PhysReg baseReg,
                             const DynLocation* keyIn,
                             const Location& outLoc) {
  bool isInline = keyIn->isInt() && !baseInput->isVariant();
  // Let the array helpers handle refcounting logic: key down,
  // return value up.
  SKTRACE(1, i.source, "emitCGetM: committed to unary load\n");
  bool decRefBase = baseInput->isStack();
  PhysReg decRefReg = noreg;
  bool stackSavedDecRef = false;
  if (decRefBase) {
    // We'll need to decref the base after the call. Make sure we hold
    // on to the value if it's a variant or from a global.
    if (isSupportedCGetM_GE(i)) {
      decRefReg = baseReg;
    } else if (baseInput->isVariant()) {
      ASSERT(!isInline);
      decRefReg = getReg(baseInput->location);
    }
    if (!isInline && decRefReg != noreg && kCallerSaved.contains(decRefReg)) {
      stackSavedDecRef = true;
      a.    pushr(decRefReg);
      a.    sub_imm32_reg64(8, unsafe_rsp);
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
  if (isInline) {
    emitHphpArrayGetIntKey(i, baseReg, *keyIn, outLoc, fptr);
  } else {
    EMIT_CALL(a, fptr,R(baseReg), V(keyIn->location), A(outLoc));
    recordReentrantCall(i);
  }
  m_regMap.invalidate(outLoc);

  if (decRefBase) {
    // For convenience of decRefs, the helpers return the ArrayData*. The
    // inline translation left baseReg intact.
    PhysReg base = isInline ? baseReg : rax;
    // but if it was boxed or from a global, we need to get the
    // original address back...
    if (decRefReg != noreg) {
      if (stackSavedDecRef) {
        ASSERT(!isInline);
        a.    add_imm32_reg64(8, unsafe_rsp);
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

void
TranslatorX64::translateCGetM(const Tracelet& t,
                              const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  ASSERT(i.outStack);

  if (isSupportedCGetM_LEE(i)) {
    Stats::emitInc(a, Stats::Tx64_CGetMLEE);
    translateCGetM_LEE(t, i);
    return;
  }
  if (isSupportedCGetM_GE(i)) {
    Stats::emitInc(a, Stats::Tx64_CGetMGE);
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
    Stats::emitInc(a, Stats::Tx64_CGetMArray);
    emitArrayElem(i, &base, baseReg, &key, i.outStack->location);
    return;
  }
  Stats::emitInc(a, Stats::Tx64_CGetMGeneric);
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

  LazyScratchReg scratch(m_regMap);
  if (base.isVariant()) {
    scratch.alloc();
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
  ASSERT(ni.inputs.size() >= 1);
  ASSERT(ni.outStack && !ni.outLocal);
  if (!ni.preppedByRef) {
    translateCGetM(t, ni);
  } else {
    translateVGetM(t, ni);
  }
}

}}}
