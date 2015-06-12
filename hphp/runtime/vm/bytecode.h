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

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/arena.h"

#include <type_traits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct Func;
struct Resumable;

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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
  const bool m_global;

 public:
  template<class F> void scan(F& mark) const {
    mark(m_nvTable);
    // TODO #6511877 scan ExtraArgs. requires calculating numExtra
    //mark(m_extraArgs);
  }

 public:
  explicit VarEnv();
  explicit VarEnv(ActRec* fp, ExtraArgs* eArgs);
  explicit VarEnv(const VarEnv* varEnv, ActRec* fp);
  ~VarEnv();

  // Free the VarEnv and locals for the given frame
  // which must have a VarEnv
  static void deallocate(ActRec* fp);

  // Allocates a local VarEnv and attaches it to the existing FP.
  static VarEnv* createLocal(ActRec* fp);

  // Allocate a global VarEnv.  Initially not attached to any frame.
  static void createGlobal();

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
 * Action taken to handle any extra arguments passed for a function call.
 */
enum class ExtraArgsAction {
  None,     // no extra arguments; zero out m_extraArgs
  Discard,  // discard extra arguments
  Variadic, // populate `...$args' parameter
  MayUseVV, // create ExtraArgs
  VarAndVV, // both of the above
};

inline ExtraArgsAction extra_args_action(const Func* func, uint32_t argc) {
  using Action = ExtraArgsAction;

  auto const nparams = func->numNonVariadicParams();
  if (argc <= nparams) return Action::None;

  if (LIKELY(func->discardExtraArgs())) {
    return Action::Discard;
  }
  if (func->attrs() & AttrMayUseVV) {
    return func->hasVariadicCaptureParam() ? Action::VarAndVV
                                           : Action::MayUseVV;
  }
  assertx(func->hasVariadicCaptureParam());
  return Action::Variadic;
}

///////////////////////////////////////////////////////////////////////////////

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
      ActRec* m_sfp;         // Previous hardware frame pointer/ActRec.
      uint64_t m_savedRip;   // In-TC address to return to.
    };
  };
  union {
    TypedValue _dummyB;
    struct {
      const Func* m_func;  // Function.
      uint32_t m_soff;         // Saved offset of caller from beginning of
                               //   caller's Func's bytecode.

      // Bits 0-28 are the number of function args.  Bits 29-31 are values from
      // the Flags enum.
      uint32_t m_numArgsAndFlags;
    };
  };
  union {
    TypedValue m_r;          // Return value teleported here when the ActRec
                             //   is post-live.
    struct {
      union {
        ObjectData* m_this;    // This.
        Class* m_cls;          // Late bound class.
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
  void setJitReturn(void* addr);
  void setReturnVMExit();

  // skip this frame if it is for a builtin function
  bool skipFrame() const;

  enum Flags : uint32_t {
    None          = 0,

    // This bit can be independently set on ActRecs with any other flag state.
    // It's used by the unwinder to know that an ActRec has been partially torn
    // down (locals freed).
    LocalsDecRefd = (1u << 29),

    // Mutually exclusive execution mode states in these 2 bits.
    InResumed     = (1u << 30),
    FromFPushCtor = (1u << 31),
    MagicDispatch = InResumed|FromFPushCtor,
  };

  static constexpr int kNumArgsBits       = 29;
  static constexpr int kNumArgsMask       = (1 << kNumArgsBits) - 1;
  static constexpr int kFlagsMask         = ~kNumArgsMask;
  static constexpr int kExecutionModeMask = ~LocalsDecRefd;

  int32_t numArgs() const { return m_numArgsAndFlags & kNumArgsMask; }
  Flags flags() const {
    return static_cast<Flags>(m_numArgsAndFlags & kFlagsMask);
  }

  bool localsDecRefd() const { return flags() & LocalsDecRefd; }
  bool resumed() const {
    return (flags() & kExecutionModeMask) == InResumed;
  }
  bool isFromFPushCtor() const {
    return (flags() & kExecutionModeMask) == FromFPushCtor;
  }
  bool magicDispatch() const {
    return (flags() & kExecutionModeMask) == MagicDispatch;
  }

  static uint32_t encodeNumArgsAndFlags(uint32_t numArgs, Flags flags) {
    assert((numArgs & ~kNumArgsMask) == 0);
    assert((uint32_t{flags} & ~kFlagsMask) == 0);
    return numArgs | flags;
  }

  void initNumArgs(uint32_t numArgs) {
    m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs, Flags::None);
  }

  void setNumArgs(uint32_t numArgs) {
    m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs, flags());
  }

  void setLocalsDecRefd() {
    m_numArgsAndFlags = m_numArgsAndFlags | LocalsDecRefd;
  }
  void setResumed() {
    assert(flags() == Flags::None);
    m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs(), InResumed);
  }
  void setFromFPushCtor() {
    assert(flags() == Flags::None);
    m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs(), FromFPushCtor);
  }

  void setMagicDispatch(StringData* invName) {
    assert(flags() == Flags::None);
    m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs(), MagicDispatch);
    m_invName = invName;
  }

  StringData* clearMagicDispatch() {
    assert(magicDispatch());
    auto const invName = getInvName();
    m_numArgsAndFlags = encodeNumArgsAndFlags(
      numArgs(),
      static_cast<Flags>(flags() & ~MagicDispatch)
    );
    trashVarEnv();
    return invName;
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

  /*
   * To conserve space, we use unions for pairs of mutually exclusive fields
   * (fields that are not used at the same time). We use unions for
   * m_this/m_cls and m_varEnv/m_invName.
   *
   * The least significant bit is used as a marker for each pair of fields so
   * that we can distinguish at runtime which field is valid. We define
   * accessors below to encapsulate this logic.
   */

  static constexpr uintptr_t kHasClassBit       = 0x1;
  static constexpr uintptr_t kExtraArgsBit      = 0x1;
  static constexpr uintptr_t kTrashedVarEnvSlot = 0xfeeefeee000f000f;

  bool hasThis() const {
    return m_this && !(reinterpret_cast<uintptr_t>(m_this) & kHasClassBit);
  }

  ObjectData* getThis() const {
    assert(hasThis());
    return m_this;
  }

  void setThis(ObjectData* val) {
    m_this = val;
  }

  bool hasClass() const {
    return reinterpret_cast<uintptr_t>(m_cls) & kHasClassBit;
  }

  Class* getClass() const {
    assert(hasClass());
    return reinterpret_cast<Class*>(
      reinterpret_cast<uintptr_t>(m_cls) & ~kHasClassBit);
  }

  void setClass(Class* val) {
    m_cls = reinterpret_cast<Class*>(
      reinterpret_cast<uintptr_t>(val) | kHasClassBit);
  }

  void trashVarEnv() {
    if (debug) setVarEnv(reinterpret_cast<VarEnv*>(kTrashedVarEnvSlot));
  }

  bool checkVarEnv() const {
    always_assert(m_varEnv != reinterpret_cast<VarEnv*>(kTrashedVarEnvSlot));
    return true;
  }

  bool hasVarEnv() const {
    assert(checkVarEnv());
    assert(!magicDispatch());
    return m_varEnv && !(reinterpret_cast<uintptr_t>(m_varEnv) & kExtraArgsBit);
  }

  bool hasExtraArgs() const {
    assert(checkVarEnv());
    return reinterpret_cast<uintptr_t>(m_extraArgs) & kExtraArgsBit;
  }

  VarEnv* getVarEnv() const {
    assert(hasVarEnv());
    return m_varEnv;
  }

  StringData* getInvName() const {
    assert(magicDispatch());
    assert(checkVarEnv());
    return m_invName;
  }

  ExtraArgs* getExtraArgs() const {
    if (!hasExtraArgs()) return nullptr;
    return reinterpret_cast<ExtraArgs*>(
      reinterpret_cast<uintptr_t>(m_extraArgs) & ~kExtraArgsBit);
  }

  void setVarEnv(VarEnv* val) {
    m_varEnv = val;
  }

  void setExtraArgs(ExtraArgs* val) {
    m_extraArgs = reinterpret_cast<ExtraArgs*>(
      reinterpret_cast<uintptr_t>(val) | kExtraArgsBit);
  }

  TypedValue* getExtraArg(unsigned ind) const {
    assert(hasExtraArgs() || hasVarEnv());
    return hasExtraArgs() ? getExtraArgs()->getExtraArg(ind) :
           hasVarEnv()    ? getVarEnv()->getExtraArg(ind) :
           static_cast<TypedValue*>(0);
  }
};

static_assert(offsetof(ActRec, m_sfp) == 0,
              "m_sfp should be at offset 0 of ActRec");

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns true iff ar represents a frame on the VM eval stack or a Resumable
 * object on the PHP heap.
 */
bool isVMFrame(const ActRec* ar);

/*
 * Returns true iff the given address is one of the special debugger return
 * helpers.
 */
bool isDebuggerReturnHelper(void* addr);

/*
 * If ar->m_savedRip points somewhere in the TC that is not a return helper,
 * change it to point to an appropriate return helper. The two different
 * versions are for the different needs of the C++ unwinder and debugger hooks,
 * respectively.
 */
void unwindPreventReturnToTC(ActRec* ar);
void debuggerPreventReturnToTC(ActRec* ar);

/*
 * Call debuggerPreventReturnToTC() on all live VM frames in this thread.
 */
void debuggerPreventReturnsToTC();

///////////////////////////////////////////////////////////////////////////////

inline int32_t arOffset(const ActRec* ar, const ActRec* other) {
  return (intptr_t(other) - intptr_t(ar)) / sizeof(TypedValue);
}

inline ActRec* arAtOffset(const ActRec* ar, int32_t offset) {
  return (ActRec*)(intptr_t(ar) + intptr_t(offset * sizeof(TypedValue)));
}

inline ActRec* arFromSpOffset(const ActRec *sp, int32_t offset) {
  return arAtOffset(sp, offset);
}

void frame_free_locals_no_hook(ActRec* fp);

inline TypedValue* arReturn(ActRec* ar, Variant&& value) {
  frame_free_locals_no_hook(ar);
  ar->m_r = *value.asTypedValue();
  tvWriteNull(value.asTypedValue());
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

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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
  explicit Fault()
    : m_raiseNesting(kInvalidNesting),
      m_raiseFrame(nullptr),
      m_raiseOffset(kInvalidOffset),
      m_handledCount(0) {}

  template<class F> void scan(F& mark) const {
    mark(m_userException);
  }

  ObjectData* m_userException;

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
  bool isAllocated() { return m_elms != nullptr; }
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

  static TypedValue* anyFrameStackBase(const ActRec* fp);
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
    tvRefcountedDecRef(m_top);
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
    if (ar->magicDispatch()) {
      decRefStr(ar->getInvName());
    }
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

  template<DataType t, class T> void pushVal(T v) {
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<t>(v);
  }
  ALWAYS_INLINE void pushBool(bool v) { pushVal<KindOfBoolean>(v); }
  ALWAYS_INLINE void pushInt(int64_t v) { pushVal<KindOfInt64>(v); }
  ALWAYS_INLINE void pushDouble(double v) { pushVal<KindOfDouble>(v); }

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
  void replaceC(const Cell c) {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvRefcountedDecRef(m_top);
    *m_top = c;
  }

  template <DataType DT>
  ALWAYS_INLINE
  void replaceC() {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvRefcountedDecRef(m_top);
    *m_top = make_tv<DT>();
  }

  template <DataType DT, typename T>
  ALWAYS_INLINE
  void replaceC(T value) {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvRefcountedDecRef(m_top);
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
  const TypedValue* const base = Stack::anyFrameStackBase(fp);
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

void resetCoverageCounters();

// The interpOne*() methods implement individual opcode handlers.
using InterpOneFunc = jit::TCA (*) (ActRec*, TypedValue*, Offset);
extern InterpOneFunc interpOneEntryPoints[];

bool doFCallArrayTC(PC pc);
bool doFCall(ActRec* ar, PC& pc);
jit::TCA dispatchBB();
void pushLocalsAndIterators(const Func* func, int nparams = 0);
Array getDefinedVariables(const ActRec*);

///////////////////////////////////////////////////////////////////////////////

}

#endif
