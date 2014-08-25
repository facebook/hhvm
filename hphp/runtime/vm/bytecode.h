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

#ifndef incl_HPHP_VM_BYTECODE_H_
#define incl_HPHP_VM_BYTECODE_H_

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-helpers.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/name-value-table.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/arena.h"

#include <type_traits>

namespace HPHP {

/**
 * These macros allow us to easily change the arguments to iop*() opcode
 * implementations.
 */
#define IOP_ARGS        PC& pc
#define IOP_PASS_ARGS   pc
#define IOP_PASS(pc)    pc

#define EVAL_FILENAME_SUFFIX ") : eval()'d code"

ALWAYS_INLINE
void SETOP_BODY_CELL(Cell* lhs, SetOpOp op, Cell* rhs) {
  assert(cellIsPlausible(*lhs));
  assert(cellIsPlausible(*rhs));

  switch (op) {
  case SetOpOp::PlusEqual:      cellAddEq(*lhs, *rhs); return;
  case SetOpOp::MinusEqual:     cellSubEq(*lhs, *rhs); return;
  case SetOpOp::MulEqual:       cellMulEq(*lhs, *rhs); return;
  case SetOpOp::DivEqual:       cellDivEq(*lhs, *rhs); return;
  case SetOpOp::PowEqual:       cellPowEq(*lhs, *rhs); return;
  case SetOpOp::ModEqual:       cellModEq(*lhs, *rhs); return;
  case SetOpOp::ConcatEqual:
    concat_assign(tvAsVariant(lhs), cellAsCVarRef(*rhs).toString());
    return;
  case SetOpOp::AndEqual:       cellBitAndEq(*lhs, *rhs); return;
  case SetOpOp::OrEqual:        cellBitOrEq(*lhs, *rhs);  return;
  case SetOpOp::XorEqual:       cellBitXorEq(*lhs, *rhs); return;
  case SetOpOp::SlEqual:        cellShlEq(*lhs, *rhs); return;
  case SetOpOp::SrEqual:        cellShrEq(*lhs, *rhs); return;
  case SetOpOp::PlusEqualO:     cellAddEqO(*lhs, *rhs); return;
  case SetOpOp::MinusEqualO:    cellSubEqO(*lhs, *rhs); return;
  case SetOpOp::MulEqualO:      cellMulEqO(*lhs, *rhs); return;
  }
  not_reached();
}

ALWAYS_INLINE
void SETOP_BODY(TypedValue* lhs, SetOpOp op, Cell* rhs) {
  SETOP_BODY_CELL(tvToCell(lhs), op, rhs);
}

class Func;
struct ActRec;

struct ExtraArgs : private boost::noncopyable {
  /*
   * Allocate an ExtraArgs structure, with arguments copied from the
   * evaluation stack.  This takes ownership of the args without
   * adjusting reference counts, so they must be discarded from the
   * stack.
   */
  static ExtraArgs* allocateCopy(TypedValue* args, unsigned nargs);

  /*
   * Allocate an ExtraArgs, without initializing any of the arguments.
   * All arguments must be initialized via getExtraArg before
   * deallocate() is called for the returned pointer.
   */
  static ExtraArgs* allocateUninit(unsigned nargs);

  /*
   * Deallocate an extraArgs structure.  Either use the one that
   * exists in a ActRec, or do it explicitly.
   */
  static void deallocate(ActRec*);
  static void deallocate(ExtraArgs*, unsigned numArgs);

  /**
   * Make a copy of ExtraArgs.
   */
  ExtraArgs* clone(ActRec* fp) const;

  /*
   * Get the slot for extra arg i, where i = argNum - func->numParams.
   */
  TypedValue* getExtraArg(unsigned argInd) const;

private:
  ExtraArgs();
  ~ExtraArgs();

  static void* allocMem(unsigned nargs);

private:
  TypedValue m_extraArgs[];
};

/*
 * Variable environment.
 *
 * A variable environment consists of the locals for the current function
 * (either pseudo-main, global function, or method), plus any variables that
 * are dynamically defined.
 *
 * Logically, a global function or method starts off with a variable
 * environment that contains only its locals, but a pseudo-main is handed
 * its caller's existing variable environment. Generally, however, we don't
 * create a variable environment for global functions or methods until it
 * actually needs one (i.e. if it is about to include a pseudo-main, or if
 * it uses dynamic variable lookups).
 *
 * Named locals always appear in the expected place on the stack, even after
 * a VarEnv is attached. Internally uses a NameValueTable to hook up names to
 * the local locations.
 */
class VarEnv {
 private:
  NameValueTable m_nvTable;
  ExtraArgs* m_extraArgs;
  uint16_t m_depth;
  bool m_global;

 public:
  explicit VarEnv();
  explicit VarEnv(ActRec* fp, ExtraArgs* eArgs);
  explicit VarEnv(const VarEnv* varEnv, ActRec* fp);
  ~VarEnv();

  // Allocates a local VarEnv and attaches it to the existing FP.
  static VarEnv* createLocal(ActRec* fp);

  // Allocate a global VarEnv.  Initially not attached to any frame.
  static VarEnv* createGlobal();

  VarEnv* clone(ActRec* fp) const;

  void suspend(const ActRec* oldFP, ActRec* newFP);
  void enterFP(ActRec* oldFP, ActRec* newFP);
  void exitFP(ActRec* fp);

  void set(const StringData* name, const TypedValue* tv);
  void bind(const StringData* name, TypedValue* tv);
  void setWithRef(const StringData* name, TypedValue* tv);
  TypedValue* lookup(const StringData* name);
  TypedValue* lookupAdd(const StringData* name);
  bool unset(const StringData* name);

  Array getDefinedVariables() const;

  // Used for save/store m_cfp for debugger
  ActRec* getFP() const { return m_nvTable.getFP(); }
  bool isGlobalScope() const { return m_global; }

  // Access to wrapped ExtraArgs, if we have one.
  TypedValue* getExtraArg(unsigned argInd) const;
};

/*
 * An "ActRec" is a call activation record. The ordering of the fields assumes
 * that stacks grow toward lower addresses.
 *
 * For most purposes, an ActRec can be considered to be in one of three
 * possible states:
 *   Pre-live:
 *     After the FPush* instruction which materialized the ActRec on the stack
 *     but before the corresponding FCall instruction
 *   Live:
 *     After the corresponding FCall instruction but before the ActRec fields
 *     and locals/iters have been decref'd (either by return or unwinding)
 *   Post-live:
 *     After the ActRec fields and locals/iters have been decref'd
 *
 * Note that when a function is invoked by the runtime via invokeFunc(), the
 * "pre-live" state is skipped and the ActRec is materialized in the "live"
 * state.
 */
struct ActRec {
  union {
    // This pair of uint64_t's must be the first two elements in the structure
    // so that the pointer to the ActRec can also be used for RBP chaining.
    // Note that ActRec's are also x64 frames, so this is an implicit machine
    // dependency.
    TypedValue _dummyA;
    struct {
      union {
        ActRec* m_sfp;         // Previous hardware frame pointer/ActRec.
        uint64_t m_savedRbp;   // TODO: Remove. Used by debugger macros.
      };
      uint64_t m_savedRip;     // In-TC address to return to.
    };
  };
  union {
    TypedValue _dummyB;
    struct {
      const Func* m_func;  // Function.
      uint32_t m_soff;         // Saved offset of caller from beginning of
                               //   caller's Func's bytecode.

      // Bits 0-28 are the number of function args.
      // Bit 29 is whether the locals were already decrefd (used by unwinder)
      // Bit 30 is whether this ActRec is embedded in a Resumable object.
      // Bit 31 is whether this ActRec came from FPushCtor*.
      uint32_t m_numArgsAndFlags;
    };
  };
  union {
    TypedValue m_r;          // Return value teleported here when the ActRec
                             //   is post-live.
    struct {
      union {
        ObjectData* m_this;    // This.
        Class* m_cls;      // Late bound class.
      };
      union {
        VarEnv* m_varEnv;       // Variable environment; only used when the
                                //   ActRec is live.
        ExtraArgs* m_extraArgs; // Light-weight extra args; used only when the
                                //   ActRec is live.
        StringData* m_invName;  // Invoked function name (used for __call);
                                //   only used when ActRec is pre-live.
      };
    };
  };

  // Get the next outermost VM frame, but if this is
  // a re-entry frame, return nullptr
  ActRec* sfp() const;

  void setReturn(ActRec* fp, PC pc, void* retAddr);
  void setReturnVMExit();

  // skip this frame if it is for a builtin function
  bool skipFrame() const;

  /**
   * Accessors for the packed m_numArgsAndFlags field. We track
   * whether ActRecs came from FPushCtor* so that during unwinding we
   * can set the flag not to call destructors for objects whose
   * constructors exit via an exception.
   */

  static constexpr int kNumArgsBits = 29;
  static constexpr int kNumArgsMask = (1 << kNumArgsBits) - 1;
  static constexpr int kFlagsMask   = ~kNumArgsMask;

  static constexpr int kLocalsDecRefdShift = kNumArgsBits;
  static constexpr int kResumedShift       = kNumArgsBits + 1;
  static constexpr int kFPushCtorShift     = kNumArgsBits + 2;

  static_assert(kFPushCtorShift <= 8 * sizeof(int32_t) - 1,
                "Out of bits in ActRec");

  static constexpr int kLocalsDecRefdMask = 1 << kLocalsDecRefdShift;
  static constexpr int kResumedMask       = 1 << kResumedShift;
  static constexpr int kFPushCtorMask     = 1 << kFPushCtorShift;

  int32_t numArgs() const {
    return m_numArgsAndFlags & kNumArgsMask;
  }

  bool localsDecRefd() const {
    return m_numArgsAndFlags & kLocalsDecRefdMask;
  }

  bool resumed() const {
    return m_numArgsAndFlags & kResumedMask;
  }

  void setResumed() {
    m_numArgsAndFlags |= kResumedMask;
  }

  bool isFromFPushCtor() const {
    return m_numArgsAndFlags & kFPushCtorMask;
  }

  static inline uint32_t
  encodeNumArgs(uint32_t numArgs, bool localsDecRefd, bool resumed,
                bool isFPushCtor) {
    assert((numArgs & kFlagsMask) == 0);
    return numArgs |
      (localsDecRefd << kLocalsDecRefdShift) |
      (resumed       << kResumedShift) |
      (isFPushCtor   << kFPushCtorShift);
  }

  void initNumArgs(uint32_t numArgs) {
    m_numArgsAndFlags = encodeNumArgs(numArgs, false, false, false);
  }

  void initNumArgsFromResumable(uint32_t numArgs) {
    m_numArgsAndFlags = encodeNumArgs(numArgs, false, true, false);
  }

  void initNumArgsFromFPushCtor(uint32_t numArgs) {
    m_numArgsAndFlags = encodeNumArgs(numArgs, false, false, true);
  }

  void setNumArgs(uint32_t numArgs) {
    m_numArgsAndFlags = encodeNumArgs(numArgs, localsDecRefd(), resumed(),
                                      isFromFPushCtor());
  }

  void setLocalsDecRefd() {
    assert(!localsDecRefd());
    m_numArgsAndFlags |= kLocalsDecRefdMask;
  }

  static void* encodeThis(ObjectData* obj, Class* cls) {
    if (obj) return obj;
    if (cls) return (char*)cls + 1;
    not_reached();
  }

  static void* encodeThis(ObjectData* obj) { return obj; }
  static void* encodeClass(const Class* cls) {
    return cls ? (char*)cls + 1 : nullptr;
  }
  static ObjectData* decodeThis(void* p) {
    return (uintptr_t(p) & 1) ? nullptr : (ObjectData*)p;
  }
  static Class* decodeClass(void* p) {
    return (uintptr_t(p) & 1) ? (Class*)(uintptr_t(p)&~1LL) : nullptr;
  }

  void setThisOrClass(void* objOrCls) {
    setThisOrClassAllowNull(objOrCls);
    assert(hasThis() || hasClass());
  }
  void setThisOrClassAllowNull(void* objOrCls) {
    m_this = (ObjectData*)objOrCls;
  }

  void* getThisOrClass() const {
    return m_this;
  }

  const Unit* unit() const {
    func()->validate();
    return func()->unit();
  }

  const Func* func() const {
    return m_func;
  }

  /**
   * To conserve space, we use unions for pairs of mutually exclusive
   * fields (fields that are not used at the same time). We use unions
   * for m_this/m_cls and m_varEnv/m_invName.
   *
   * The least significant bit is used as a marker for each pair of fields
   * so that we can distinguish at runtime which field is valid. We define
   * accessors below to encapsulate this logic.
   *
   * Note that m_invName is only used when the ActRec is pre-live. Thus when
   * an ActRec is live it is safe to directly access m_varEnv without using
   * accessors.
   */

  static constexpr int8_t kHasClassBit = 0x1;
  static constexpr int8_t kClassMask   = ~kHasClassBit;

  inline bool hasThis() const {
    return m_this && !(reinterpret_cast<intptr_t>(m_this) & kHasClassBit);
  }
  inline ObjectData* getThis() const {
    assert(hasThis());
    return m_this;
  }
  inline void setThis(ObjectData* val) {
    m_this = val;
  }
  inline bool hasClass() const {
    return reinterpret_cast<intptr_t>(m_cls) & kHasClassBit;
  }
  inline Class* getClass() const {
    assert(hasClass());
    return reinterpret_cast<Class*>(
      reinterpret_cast<intptr_t>(m_cls) & kClassMask);
  }
  inline void setClass(Class* val) {
    m_cls = reinterpret_cast<Class*>(
      reinterpret_cast<intptr_t>(val) | kHasClassBit);
  }

  // Note that reordering these is likely to require changes to the translator.
  static constexpr int8_t kInvNameBit    = 0x1;
  static constexpr int8_t kInvNameMask   = ~kInvNameBit;
  static constexpr int8_t kExtraArgsBit  = 0x2;
  static constexpr int8_t kExtraArgsMask = ~kExtraArgsBit;

  inline bool hasVarEnv() const {
    return m_varEnv &&
      !(reinterpret_cast<intptr_t>(m_varEnv) & (kInvNameBit | kExtraArgsBit));
  }
  inline bool hasInvName() const {
    return reinterpret_cast<intptr_t>(m_invName) & kInvNameBit;
  }
  inline bool hasExtraArgs() const {
    return reinterpret_cast<intptr_t>(m_extraArgs) & kExtraArgsBit;
  }
  inline VarEnv* getVarEnv() const {
    assert(hasVarEnv());
    return m_varEnv;
  }
  inline StringData* getInvName() const {
    assert(hasInvName());
    return reinterpret_cast<StringData*>(
      reinterpret_cast<intptr_t>(m_invName) & kInvNameMask);
  }
  inline ExtraArgs* getExtraArgs() const {
    return reinterpret_cast<ExtraArgs*>(
      reinterpret_cast<intptr_t>(m_extraArgs) & kExtraArgsMask);
  }
  inline void setVarEnv(VarEnv* val) {
    m_varEnv = val;
  }
  inline void setInvName(StringData* val) {
    m_invName = reinterpret_cast<StringData*>(
      reinterpret_cast<intptr_t>(val) | kInvNameBit);
  }
  inline void setExtraArgs(ExtraArgs* val) {
    m_extraArgs = reinterpret_cast<ExtraArgs*>(
      reinterpret_cast<intptr_t>(val) | kExtraArgsBit);
  }

  // Accessors for extra arg queries.
  TypedValue* getExtraArg(unsigned ind) const {
    assert(hasExtraArgs() || hasVarEnv());
    return hasExtraArgs() ? getExtraArgs()->getExtraArg(ind) :
           hasVarEnv()    ? getVarEnv()->getExtraArg(ind) :
           static_cast<TypedValue*>(0);
  }
};

static_assert(offsetof(ActRec, m_sfp) == 0,
              "m_sfp should be at offset 0 of ActRec");

inline int32_t arOffset(const ActRec* ar, const ActRec* other) {
  return (intptr_t(other) - intptr_t(ar)) / sizeof(TypedValue);
}

inline ActRec* arAtOffset(const ActRec* ar, int32_t offset) {
  return (ActRec*)(intptr_t(ar) + intptr_t(offset * sizeof(TypedValue)));
}

inline ActRec* arFromSpOffset(const ActRec *sp, int32_t offset) {
  return arAtOffset(sp, offset);
}

inline TypedValue* arReturn(ActRec* ar, const Variant& value) {
  ar->m_r = *value.asTypedValue();
  return &ar->m_r;
}

template <bool crossBuiltin> Class* arGetContextClassImpl(const ActRec* ar);
template <> Class* arGetContextClassImpl<true>(const ActRec* ar);
template <> Class* arGetContextClassImpl<false>(const ActRec* ar);
inline Class* arGetContextClass(const ActRec* ar) {
  return arGetContextClassImpl<false>(ar);
}
inline Class* arGetContextClassFromBuiltin(const ActRec* ar) {
  return arGetContextClassImpl<true>(ar);
}

// Used by extension functions that take a PHP "callback", since they need to
// figure out the callback context once and call it multiple times. (e.g.
// array_map, array_filter, ...)
struct CallCtx {
  const Func* func;
  ObjectData* this_;
  Class* cls;
  StringData* invName;
};

constexpr size_t kNumIterCells = sizeof(Iter) / sizeof(Cell);
constexpr size_t kNumActRecCells = sizeof(ActRec) / sizeof(Cell);

/*
 * We pad all stack overflow checks by a small amount to allow for three
 * things:
 *
 *   - inlining functions without having to either do another stack
 *     check (or chase down prologues to smash checks to be bigger).
 *
 *   - omitting stack overflow checks on leaf functions
 *
 *   - delaying stack overflow checks on reentry
 */
constexpr int kStackCheckLeafPadding = 20;
constexpr int kStackCheckReenterPadding = 9;
constexpr int kStackCheckPadding = kStackCheckLeafPadding +
  kStackCheckReenterPadding;

constexpr int kInvalidRaiseLevel = -1;
constexpr int kInvalidNesting = -1;

struct Fault {
  enum class Type : int16_t {
    UserException,
    CppException
  };

  explicit Fault()
    : m_raiseNesting(kInvalidNesting),
      m_raiseFrame(nullptr),
      m_raiseOffset(kInvalidOffset),
      m_handledCount(0) {}

  union {
    ObjectData* m_userException;
    Exception* m_cppException;
  };
  Type m_faultType;

  // The VM nesting at the moment where the exception was thrown.
  int m_raiseNesting;
  // The frame where the exception was thrown.
  ActRec* m_raiseFrame;
  // The offset within the frame where the exception was thrown.
  // This value is updated when a fault is updated when exception
  // chaining takes place. In this case the raise offset of the newly
  // thrown exception is set to the offset of the previously thrown
  // exception. The offset is also updated when the exception
  // propagates outside its current frame.
  Offset m_raiseOffset;
  // The number of EHs that were already examined for this exception.
  // This is used to ensure that the same exception handler is not
  // run twice for the same exception. The unwinder may be entered
  // multiple times for the same fault as a result of calling Unwind.
  // The field is used to skip through the EHs that were already run.
  int m_handledCount;
};

// Interpreter evaluation stack.
class Stack {
  TypedValue* m_elms;
  TypedValue* m_top;
  TypedValue* m_base; // Stack grows down, so m_base is beyond the end of
                      // m_elms.

public:
  void* getStackLowAddress() const { return m_elms; }
  void* getStackHighAddress() const { return m_base; }
  bool isValidAddress(uintptr_t v) {
    return v >= uintptr_t(m_elms) && v < uintptr_t(m_base);
  }
  void requestInit();
  void requestExit();

  static const int sSurprisePageSize;
  static const unsigned sMinStackElms;
  static void ValidateStackSize();
  Stack();
  ~Stack();

  std::string toString(const ActRec* fp, int offset,
                       std::string prefix="") const;

  bool wouldOverflow(int numCells) const;

  /*
   * top --
   * topOfStackOffset --
   *
   *   Accessors for the x64 translator. Do not play on or around.
   */
  TypedValue*& top() {
    return m_top;
  }

  static constexpr size_t topOfStackOffset() {
    return offsetof(Stack, m_top);
  }

  static TypedValue* frameStackBase(const ActRec* fp);
  static TypedValue* resumableStackBase(const ActRec* fp);

  ALWAYS_INLINE
  size_t count() const {
    return ((uintptr_t)m_base - (uintptr_t)m_top) / sizeof(TypedValue);
  }

  // Same as discard(), but meant to replace popC() iff the interpreter knows
  // for certain that decrementing a refcount is unnecessary.
  ALWAYS_INLINE
  void popX() {
    assert(m_top != m_base);
    assert(!IS_REFCOUNTED_TYPE(m_top->m_type));
    tvDebugTrash(m_top);
    m_top++;
  }

  ALWAYS_INLINE
  void popC() {
    assert(m_top != m_base);
    assert(cellIsPlausible(*m_top));
    tvRefcountedDecRefCell(m_top);
    tvDebugTrash(m_top);
    m_top++;
  }

  ALWAYS_INLINE
  void popA() {
    assert(m_top != m_base);
    assert(m_top->m_type == KindOfClass);
    tvDebugTrash(m_top);
    m_top++;
  }

  ALWAYS_INLINE
  void popV() {
    assert(m_top != m_base);
    assert(refIsPlausible(*m_top));
    tvDecRefRef(m_top);
    tvDebugTrash(m_top);
    m_top++;
  }

  ALWAYS_INLINE
  void popTV() {
    assert(m_top != m_base);
    assert(m_top->m_type == KindOfClass || tvIsPlausible(*m_top));
    tvRefcountedDecRef(m_top);
    tvDebugTrash(m_top);
    m_top++;
  }

  // popAR() should only be used to tear down a pre-live ActRec. Once
  // an ActRec is live, it should be torn down using frame_free_locals()
  // followed by discardAR() or ret().
  ALWAYS_INLINE
  void popAR() {
    assert(m_top != m_base);
    ActRec* ar = (ActRec*)m_top;
    if (ar->hasThis()) decRefObj(ar->getThis());
    if (ar->hasInvName()) decRefStr(ar->getInvName());

    // This should only be used on a pre-live ActRec.
    assert(!ar->hasVarEnv());
    assert(!ar->hasExtraArgs());
    discardAR();
  }

  ALWAYS_INLINE
  void discardAR() {
    assert(m_top != m_base);
    if (debug) {
      for (int i = 0; i < kNumActRecCells; ++i) {
        tvDebugTrash(m_top + i);
      }
    }
    m_top += kNumActRecCells;
    assert((uintptr_t)m_top <= (uintptr_t)m_base);
  }

  ALWAYS_INLINE
  void ret() {
    // Leave part of the activation on the stack, since the return value now
    // resides there.
    if (debug) {
      for (int i = 0; i < kNumActRecCells - 1; ++i) {
        tvDebugTrash(m_top + i);
      }
    }
    m_top += kNumActRecCells - 1;
    assert((uintptr_t)m_top <= (uintptr_t)m_base);
  }

  ALWAYS_INLINE
  void discard() {
    assert(m_top != m_base);
    tvDebugTrash(m_top);
    m_top++;
  }

  ALWAYS_INLINE
  void ndiscard(size_t n) {
    assert((uintptr_t)&m_top[n] <= (uintptr_t)m_base);
    if (debug) {
      for (int i = 0; i < n; ++i) {
        tvDebugTrash(m_top + i);
      }
    }
    m_top += n;
  }

  ALWAYS_INLINE
  void dup() {
    assert(m_top != m_base);
    assert(m_top != m_elms);
    assert(m_top->m_type != KindOfRef);
    Cell* fr = m_top;
    m_top--;
    Cell* to = m_top;
    cellDup(*fr, *to);
  }

  ALWAYS_INLINE
  void box() {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvBox(m_top);
  }

  ALWAYS_INLINE
  void unbox() {
    assert(m_top != m_base);
    tvUnbox(m_top);
  }

  ALWAYS_INLINE
  void pushUninit() {
    assert(m_top != m_elms);
    m_top--;
    tvWriteUninit(m_top);
  }

  ALWAYS_INLINE
  void pushNull() {
    assert(m_top != m_elms);
    m_top--;
    tvWriteNull(m_top);
  }

  ALWAYS_INLINE
  void pushNullUninit() {
    assert(m_top != m_elms);
    m_top--;
    m_top->m_data.num = 0;
    m_top->m_type = KindOfUninit;
  }

  #define PUSH_METHOD(name, type, field, value)                               \
  ALWAYS_INLINE void push##name() {                                           \
    assert(m_top != m_elms);                                                  \
    m_top--;                                                                  \
    m_top->m_data.field = value;                                              \
    m_top->m_type = type;                                                     \
  }
  PUSH_METHOD(True, KindOfBoolean, num, 1)
  PUSH_METHOD(False, KindOfBoolean, num, 0)

  #define PUSH_METHOD_ARG(name, type, field, argtype, arg)                    \
  ALWAYS_INLINE void push##name(argtype arg) {                                \
    assert(m_top != m_elms);                                                  \
    m_top--;                                                                  \
    m_top->m_data.field = arg;                                                \
    m_top->m_type = type;                                                     \
  }
  PUSH_METHOD_ARG(Bool, KindOfBoolean, num, bool, b)
  PUSH_METHOD_ARG(Int, KindOfInt64, num, int64_t, i)
  PUSH_METHOD_ARG(Double, KindOfDouble, dbl, double, d)

  // This should only be called directly when the caller has
  // already adjusted the refcount appropriately
  ALWAYS_INLINE
  void pushStringNoRc(StringData* s) {
    assert(m_top != m_elms);
    m_top--;
    m_top->m_data.pstr = s;
    m_top->m_type = KindOfString;
  }

  ALWAYS_INLINE
  void pushStaticString(StringData* s) {
    assert(s->isStatic()); // No need to call s->incRefCount().
    assert(m_top != m_elms);
    m_top--;
    m_top->m_data.pstr = s;
    m_top->m_type = KindOfStaticString;
  }

  // This should only be called directly when the caller has
  // already adjusted the refcount appropriately
  ALWAYS_INLINE
  void pushArrayNoRc(ArrayData* a) {
    assert(m_top != m_elms);
    m_top--;
    m_top->m_data.parr = a;
    m_top->m_type = KindOfArray;
  }

  ALWAYS_INLINE
  void pushArray(ArrayData* a) {
    assert(a);
    pushArrayNoRc(a);
    a->incRefCount();
  }

  ALWAYS_INLINE
  void pushStaticArray(ArrayData* a) {
    assert(a->isStatic()); // No need to call a->incRefCount().
    pushArrayNoRc(a);
  }

  // This should only be called directly when the caller has
  // already adjusted the refcount appropriately
  ALWAYS_INLINE
  void pushObjectNoRc(ObjectData* o) {
    assert(m_top != m_elms);
    m_top--;
    m_top->m_data.pobj = o;
    m_top->m_type = KindOfObject;
  }

  ALWAYS_INLINE
  void pushObject(ObjectData* o) {
    pushObjectNoRc(o);
    o->incRefCount();
  }

  ALWAYS_INLINE
  void nalloc(size_t n) {
    assert((uintptr_t)&m_top[-n] <= (uintptr_t)m_base);
    m_top -= n;
  }

  ALWAYS_INLINE
  Cell* allocC() {
    assert(m_top != m_elms);
    m_top--;
    return (Cell*)m_top;
  }

  ALWAYS_INLINE
  Ref* allocV() {
    assert(m_top != m_elms);
    m_top--;
    return (Ref*)m_top;
  }

  ALWAYS_INLINE
  TypedValue* allocTV() {
    assert(m_top != m_elms);
    m_top--;
    return m_top;
  }

  ALWAYS_INLINE
  ActRec* allocA() {
    assert((uintptr_t)&m_top[-kNumActRecCells] >= (uintptr_t)m_elms);
    assert(kNumActRecCells * sizeof(Cell) == sizeof(ActRec));
    m_top -= kNumActRecCells;
    return (ActRec*)m_top;
  }

  ALWAYS_INLINE
  void allocI() {
    assert(kNumIterCells * sizeof(Cell) == sizeof(Iter));
    assert((uintptr_t)&m_top[-kNumIterCells] >= (uintptr_t)m_elms);
    m_top -= kNumIterCells;
  }

  ALWAYS_INLINE
  void replaceC(const Cell& c) {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvRefcountedDecRefCell(m_top);
    *m_top = c;
  }

  template <DataType DT>
  ALWAYS_INLINE
  void replaceC() {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvRefcountedDecRefCell(m_top);
    *m_top = make_tv<DT>();
  }

  template <DataType DT, typename T>
  ALWAYS_INLINE
  void replaceC(T value) {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvRefcountedDecRefCell(m_top);
    *m_top = make_tv<DT>(value);
  }

  ALWAYS_INLINE
  void replaceTV(const TypedValue& tv) {
    assert(m_top != m_base);
    tvRefcountedDecRef(m_top);
    *m_top = tv;
  }

  template <DataType DT>
  ALWAYS_INLINE
  void replaceTV() {
    assert(m_top != m_base);
    tvRefcountedDecRef(m_top);
    *m_top = make_tv<DT>();
  }

  template <DataType DT, typename T>
  ALWAYS_INLINE
  void replaceTV(T value) {
    assert(m_top != m_base);
    tvRefcountedDecRef(m_top);
    *m_top = make_tv<DT>(value);
  }

  ALWAYS_INLINE
  Cell* topC() {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    return (Cell*)m_top;
  }

  ALWAYS_INLINE
  Ref* topV() {
    assert(m_top != m_base);
    assert(m_top->m_type == KindOfRef);
    return (Ref*)m_top;
  }

  ALWAYS_INLINE
  const Class* topA() {
    assert(m_top != m_base);
    assert(m_top->m_type == KindOfClass);
    return m_top->m_data.pcls;
  }

  ALWAYS_INLINE
  TypedValue* topTV() {
    assert(m_top != m_base);
    return m_top;
  }

  ALWAYS_INLINE
  Cell* indC(size_t ind) {
    assert(m_top != m_base);
    assert(m_top[ind].m_type != KindOfRef);
    return (Cell*)(&m_top[ind]);
  }

  ALWAYS_INLINE
  TypedValue* indTV(size_t ind) {
    assert(m_top != m_base);
    return &m_top[ind];
  }

  ALWAYS_INLINE
  void pushClass(Class* clss) {
    assert(m_top != m_elms);
    m_top--;
    m_top->m_data.pcls = clss;
    m_top->m_type = KindOfClass;
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * Visit all the slots and pre-live ActRecs on a live eval stack,
 * handling FPI regions and resumables correctly, and stopping when we
 * reach the supplied activation record.
 *
 * The stack elements are visited from lower address to higher, with
 * ActRecs visited after the stack slots below them.
 *
 * This will not read the VM registers (pc, fp, sp), so it will
 * perform the requested visitation independent of modifications to
 * the VM stack or frame pointer.
 */
template<class MaybeConstTVPtr, class ARFun, class TVFun>
typename std::enable_if<
  std::is_same<MaybeConstTVPtr,const TypedValue*>::value ||
  std::is_same<MaybeConstTVPtr,      TypedValue*>::value
>::type
visitStackElems(const ActRec* const fp,
                MaybeConstTVPtr const stackTop,
                Offset const bcOffset,
                ARFun arFun,
                TVFun tvFun) {
  const TypedValue* const base =
    fp->resumed() ? Stack::resumableStackBase(fp)
                  : Stack::frameStackBase(fp);
  MaybeConstTVPtr cursor = stackTop;
  assert(cursor <= base);

  if (auto fe = fp->m_func->findFPI(bcOffset)) {
    for (;;) {
      ActRec* ar;
      if (!fp->resumed()) {
        ar = arAtOffset(fp, -fe->m_fpOff);
      } else {
        // fp is pointing into the Resumable struct. Since fpOff is
        // given as an offset from the frame pointer as if it were in
        // the normal place on the main stack, we have to reconstruct
        // that "normal place".
        auto const fakePrevFP = reinterpret_cast<const ActRec*>(
          base + fp->m_func->numSlotsInFrame()
        );
        ar = arAtOffset(fakePrevFP, -fe->m_fpOff);
      }

      assert(cursor <= reinterpret_cast<TypedValue*>(ar));
      while (cursor < reinterpret_cast<TypedValue*>(ar)) {
        tvFun(cursor++);
      }
      arFun(ar);

      cursor += kNumActRecCells;
      if (fe->m_parentIndex == -1) break;
      fe = &fp->m_func->fpitab()[fe->m_parentIndex];
    }
  }

  while (cursor < base) {
    tvFun(cursor++);
  }
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
