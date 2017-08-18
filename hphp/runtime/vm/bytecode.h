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

#ifndef incl_HPHP_VM_BYTECODE_H_
#define incl_HPHP_VM_BYTECODE_H_

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/name-value-table.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/arena.h"
#include "hphp/util/type-traits.h"

#include <type_traits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct Func;
struct Resumable;

///////////////////////////////////////////////////////////////////////////////

#define EVAL_FILENAME_SUFFIX ") : eval()'d code"

// perform the set(op) operation on lhs & rhs, leaving the result in lhs.
// The old value of lhs is decrefed. Caller must call tvToCell() if lhs or
// rhs might be a ref.
ALWAYS_INLINE
void setopBody(Cell* lhs, SetOpOp op, Cell* rhs) {
  assert(cellIsPlausible(*lhs));
  assert(cellIsPlausible(*rhs));

  switch (op) {
  case SetOpOp::PlusEqual:      cellAddEq(*lhs, *rhs); return;
  case SetOpOp::MinusEqual:     cellSubEq(*lhs, *rhs); return;
  case SetOpOp::MulEqual:       cellMulEq(*lhs, *rhs); return;
  case SetOpOp::DivEqual:       cellDivEq(*lhs, *rhs); return;
  case SetOpOp::PowEqual:       cellPowEq(*lhs, *rhs); return;
  case SetOpOp::ModEqual:       cellModEq(*lhs, *rhs); return;
  case SetOpOp::ConcatEqual:    cellConcatEq(*lhs, *rhs); return;
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

///////////////////////////////////////////////////////////////////////////////

struct ExtraArgs {
  ExtraArgs(const ExtraArgs&) = delete;
  ExtraArgs& operator=(const ExtraArgs&) = delete;

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
  TypedValue m_extraArgs[0];
  TYPE_SCAN_FLEXIBLE_ARRAY_FIELD(m_extraArgs);
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
struct VarEnv {
 private:
  NameValueTable m_nvTable;
  ExtraArgs* m_extraArgs;
  uint16_t m_depth;
  const bool m_global;

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

#define arReturn(a, x)                          \
  ([&] {                                        \
    ActRec* ar_ = (a);                          \
    TypedValue val_;                            \
    new (&val_) Variant(x);                     \
    frame_free_locals_no_hook(ar_);             \
    tvCopy(val_, *ar_->retSlot());              \
    return ar_->retSlot();                      \
  }())

#define tvReturn(x)                                                     \
  ([&] {                                                                \
    TypedValue val_;                                                    \
    new (&val_) Variant(x);                                             \
    assert(val_.m_type != KindOfRef && val_.m_type != KindOfUninit);    \
    return val_;                                                        \
  }())

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

constexpr size_t clsRefCountToCells(size_t n) {
  return (n * sizeof(LowPtr<Class>*) + sizeof(Cell) - 1) / sizeof(Cell);
}

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
struct Stack {
private:
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
    assert(!isRefcountedType(m_top->m_type));
    tvDebugTrash(m_top);
    m_top++;
  }

  ALWAYS_INLINE
  void popC() {
    assert(m_top != m_base);
    assert(cellIsPlausible(*m_top));
    tvDecRefGen(m_top);
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
  void popU() {
    assert(m_top != m_base);
    assert(m_top->m_type == KindOfUninit);
    tvDebugTrash(m_top);
    ++m_top;
  }

  ALWAYS_INLINE
  void popTV() {
    assert(m_top != m_base);
    assert(tvIsPlausible(*m_top));
    tvDecRefGen(m_top);
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
    if (ar->func()->cls() && ar->hasThis()) decRefObj(ar->getThis());
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
  void trim(Cell* c) {
    assert(c <= m_base);
    assert(m_top <= c);
    if (debug) {
      while (m_top < c) tvDebugTrash(m_top++);
    } else {
      m_top = c;
    }
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
    *m_top = make_tv<KindOfString>(s);
  }

  ALWAYS_INLINE
  void pushStaticString(const StringData* s) {
    assert(s->isStatic()); // No need to call s->incRefCount().
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfPersistentString>(s);
  }

  // These should only be called directly when the caller has
  // already adjusted the refcount appropriately
  ALWAYS_INLINE
  void pushArrayNoRc(ArrayData* a) {
    assert(a->isPHPArray());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfArray>(a);
  }

  ALWAYS_INLINE
  void pushVecNoRc(ArrayData* a) {
    assert(a->isVecArray());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfVec>(a);
  }

  ALWAYS_INLINE
  void pushDictNoRc(ArrayData* a) {
    assert(a->isDict());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfDict>(a);
  }

  ALWAYS_INLINE
  void pushKeysetNoRc(ArrayData* a) {
    assert(a->isKeyset());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfKeyset>(a);
  }

  ALWAYS_INLINE
  void pushArray(ArrayData* a) {
    assert(a);
    pushArrayNoRc(a);
    a->incRefCount();
  }

  ALWAYS_INLINE
  void pushVec(ArrayData* a) {
    assert(a);
    pushVecNoRc(a);
    a->incRefCount();
  }

  ALWAYS_INLINE
  void pushDict(ArrayData* a) {
    assert(a);
    pushDictNoRc(a);
    a->incRefCount();
  }

  ALWAYS_INLINE
  void pushKeyset(ArrayData* a) {
    assert(a);
    pushKeysetNoRc(a);
    a->incRefCount();
  }

  ALWAYS_INLINE
  void pushStaticArray(const ArrayData* a) {
    assert(a->isStatic()); // No need to call a->incRefCount().
    assert(a->isPHPArray());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfPersistentArray>(a);
  }

  ALWAYS_INLINE
  void pushStaticVec(const ArrayData* a) {
    assert(a->isStatic()); // No need to call a->incRefCount().
    assert(a->isVecArray());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfPersistentVec>(a);
  }

  ALWAYS_INLINE
  void pushStaticDict(const ArrayData* a) {
    assert(a->isStatic()); // No need to call a->incRefCount().
    assert(a->isDict());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfPersistentDict>(a);
  }

  ALWAYS_INLINE
  void pushStaticKeyset(const ArrayData* a) {
    assert(a->isStatic()); // No need to call a->incRefCount().
    assert(a->isKeyset());
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfPersistentKeyset>(a);
  }

  // This should only be called directly when the caller has
  // already adjusted the refcount appropriately
  ALWAYS_INLINE
  void pushObjectNoRc(ObjectData* o) {
    assert(m_top != m_elms);
    m_top--;
    *m_top = make_tv<KindOfObject>(o);
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
  void allocClsRefSlots(size_t n) {
    assert((uintptr_t)&m_top[-clsRefCountToCells(n)] >= (uintptr_t)m_elms);
    m_top -= clsRefCountToCells(n);
    if (debug) {
      memset(m_top, kTrashClsRef, clsRefCountToCells(n) * sizeof(Cell));
    }
  }

  ALWAYS_INLINE
  void replaceC(const Cell c) {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvDecRefGen(m_top);
    *m_top = c;
  }

  template <DataType DT>
  ALWAYS_INLINE
  void replaceC() {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvDecRefGen(m_top);
    *m_top = make_tv<DT>();
  }

  template <DataType DT, typename T>
  ALWAYS_INLINE
  void replaceC(T value) {
    assert(m_top != m_base);
    assert(m_top->m_type != KindOfRef);
    tvDecRefGen(m_top);
    *m_top = make_tv<DT>(value);
  }

  ALWAYS_INLINE
  void replaceTV(const TypedValue& tv) {
    assert(m_top != m_base);
    tvDecRefGen(m_top);
    *m_top = tv;
  }

  template <DataType DT>
  ALWAYS_INLINE
  void replaceTV() {
    assert(m_top != m_base);
    tvDecRefGen(m_top);
    *m_top = make_tv<DT>();
  }

  template <DataType DT, typename T>
  ALWAYS_INLINE
  void replaceTV(T value) {
    assert(m_top != m_base);
    tvDecRefGen(m_top);
    *m_top = make_tv<DT>(value);
  }

  ALWAYS_INLINE
  Cell* topC() {
    assert(m_top != m_base);
    return tvAssertCell(m_top);
  }

  ALWAYS_INLINE
  Ref* topV() {
    assert(m_top != m_base);
    assert(m_top->m_type == KindOfRef);
    return (Ref*)m_top;
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
    return tvAssertCell(&m_top[ind]);
  }

  ALWAYS_INLINE
  TypedValue* indTV(size_t ind) {
    assert(m_top != m_base);
    return &m_top[ind];
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
template<class TV, class ARFun, class TVFun>
typename maybe_const<TV, TypedValue>::type
visitStackElems(const ActRec* const fp,
                TV* const stackTop,
                Offset const bcOffset,
                ARFun arFun,
                TVFun tvFun) {
  const TypedValue* const base = Stack::anyFrameStackBase(fp);
  auto cursor = stackTop;
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
      arFun(ar, fe->m_fpushOff);

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

bool doFCallArrayTC(PC pc, int32_t numArgs, void*);
bool doFCall(ActRec* ar, PC& pc);
jit::TCA dispatchBB();
void pushFrameSlots(const Func* func, int nparams = 0);
Array getDefinedVariables(const ActRec*);
jit::TCA suspendStack(PC& pc);

enum class StackArgsState { // tells prepareFuncEntry how much work to do
  // the stack may contain more arguments than the function expects
  Untrimmed,
  // the stack has already been trimmed of any extra arguments, which
  // have been teleported away into ExtraArgs and/or a variadic param
  Trimmed
};
void enterVMAtFunc(ActRec* enterFnAr, StackArgsState stk, VarEnv* varEnv);
void enterVMAtCurPC();
bool prepareArrayArgs(ActRec* ar, const Cell args, Stack& stack,
                      int nregular, bool doCufRefParamChecks,
                      TypedValue* retval);

///////////////////////////////////////////////////////////////////////////////

}

#endif
