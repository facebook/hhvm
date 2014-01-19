/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <type_traits>
#include <boost/optional.hpp>

#include "hphp/util/util.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/name-value-table.h"
#include "hphp/runtime/vm/request-arena.h"

namespace HPHP {

/**
 * These macros allow us to easily change the arguments to iop*() opcode
 * implementations.
 */
#define IOP_ARGS        PC& pc
#define IOP_PASS_ARGS   pc
#define IOP_PASS(pc)    pc

ALWAYS_INLINE
void SETOP_BODY_CELL(Cell* lhs, SetOpOp op, Cell* rhs) {
  assert(cellIsPlausible(*lhs));
  assert(cellIsPlausible(*rhs));

  switch (op) {
  case SetOpOp::PlusEqual:      cellAddEq(*lhs, *rhs); return;
  case SetOpOp::MinusEqual:     cellSubEq(*lhs, *rhs); return;
  case SetOpOp::MulEqual:       cellMulEq(*lhs, *rhs); return;
  case SetOpOp::DivEqual:       cellDivEq(*lhs, *rhs); return;
  case SetOpOp::ModEqual:       cellModEq(*lhs, *rhs); return;
  case SetOpOp::ConcatEqual:
    concat_assign(tvAsVariant(lhs), cellAsCVarRef(*rhs).toString());
    return;
  case SetOpOp::AndEqual:       cellBitAndEq(*lhs, *rhs); return;
  case SetOpOp::OrEqual:        cellBitOrEq(*lhs, *rhs);  return;
  case SetOpOp::XorEqual:       cellBitXorEq(*lhs, *rhs); return;

  case SetOpOp::SlEqual:
    cellCastToInt64InPlace(lhs);
    lhs->m_data.num <<= cellToInt(*rhs);
    return;
  case SetOpOp::SrEqual:
    cellCastToInt64InPlace(lhs);
    lhs->m_data.num >>= cellToInt(*rhs);
    return;
  }
  not_reached();
}

ALWAYS_INLINE
void SETOP_BODY(TypedValue* lhs, SetOpOp op, Cell* rhs) {
  SETOP_BODY_CELL(tvToCell(lhs), op, rhs);
}

class Func;
struct ActRec;

// max number of arguments for direct call to builtin
const int kMaxBuiltinArgs = 5;

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
  ExtraArgs* m_extraArgs;
  uint16_t m_depth;
  bool m_malloced;
  bool m_global;
  ActRec* m_cfp;
  // TODO remove vector (#1099580).  Note: trying changing this to a
  // TinyVector<> for now increased icache misses, but maybe will be
  // feasable later (see D511561).
  std::vector<TypedValue**> m_restoreLocations;
  boost::optional<NameValueTable> m_nvTable;

 private:
  explicit VarEnv();
  explicit VarEnv(ActRec* fp, ExtraArgs* eArgs);
  VarEnv(const VarEnv&);
  VarEnv& operator=(const VarEnv&);
  ~VarEnv();

  void ensureNvt();

 public:
  /*
   * Creates a VarEnv and attaches it to the existing frame fp.
   *
   * A lazy attach works by bringing all currently existing values for
   * the names in fp->m_func into the variable environment.  This is
   * used when we need a variable environment for some caller frame
   * (because we're about to attach a callee frame using attach()) but
   * don't actually have one.
   */
  static VarEnv* createLocalOnStack(ActRec* fp);
  static VarEnv* createLocalOnHeap(ActRec* fp);

  // Allocate a global VarEnv.  Initially not attached to any frame.
  static VarEnv* createGlobal();

  static void destroy(VarEnv*);

  static size_t getObjectSz(ActRec* fp);

  void attach(ActRec* fp);
  void detach(ActRec* fp);

  void set(const StringData* name, TypedValue* tv);
  void bind(const StringData* name, TypedValue* tv);
  void setWithRef(const StringData* name, TypedValue* tv);
  TypedValue* lookup(const StringData* name);
  TypedValue* lookupAdd(const StringData* name);
  bool unset(const StringData* name);

  Array getDefinedVariables() const;

  // Used for save/store m_cfp for debugger
  void setCfp(ActRec* fp) { m_cfp = fp; }
  ActRec* getCfp() const { return m_cfp; }
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
      uint64_t m_savedRbp;     // Previous hardware frame pointer/ActRec.
      uint64_t m_savedRip;     // In-TC address to return to.
    };
  };
  union {
    TypedValue _dummyB;
    struct {
      const Func* m_func;  // Function.
      uint32_t m_soff;         // Saved offset of caller from beginning of
                               //   caller's Func's bytecode.

      // Bits 0-30 are the number of function args; the high bit is
      // whether this ActRec came from FPushCtor*.
      uint32_t m_numArgsAndCtorFlag;
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
  // a re-entry frame, return ar
  ActRec* arGetSfp() const;

  // skip this frame if it is for a builtin function
  bool skipFrame() const;

  /**
   * Accessors for the packed m_numArgsAndCtorFlag field. We track
   * whether ActRecs came from FPushCtor* so that during unwinding we
   * can set the flag not to call destructors for objects whose
   * constructors exit via an exception.
   */

  int32_t numArgs() const {
    return decodeNumArgs(m_numArgsAndCtorFlag).first;
  }

  bool isFromFPushCtor() const {
    return decodeNumArgs(m_numArgsAndCtorFlag).second;
  }

  static inline uint32_t
  encodeNumArgs(uint32_t numArgs, bool isFPushCtor = false) {
    assert((numArgs & (1u << 31)) == 0);
    return numArgs | (isFPushCtor << 31);
  }

  static inline std::pair<uint32_t,bool>
  decodeNumArgs(uint32_t numArgs) {
    return { numArgs & ~(1u << 31), numArgs & (1u << 31) };
  }

  void initNumArgs(uint32_t numArgs, bool isFPushCtor = false) {
    m_numArgsAndCtorFlag = encodeNumArgs(numArgs, isFPushCtor);
  }

  void setNumArgs(uint32_t numArgs) {
    initNumArgs(numArgs, isFromFPushCtor());
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

#define UNION_FIELD_ACCESSORS2(name1, type1, field1, name2, type2, field2) \
  inline bool has##name1() const { \
    return field1 && !(intptr_t(field1) & 1LL); \
  } \
  inline bool has##name2() const { \
    return bool(intptr_t(field2) & 1LL); \
  } \
  inline type1 get##name1() const { \
    assert(has##name1()); \
    return field1; \
  } \
  inline type2 get##name2() const { \
    assert(has##name2()); \
    return (type2)(intptr_t(field2) & ~1LL); \
  } \
  inline void set##name1(type1 val) { \
    field1 = val; \
  } \
  inline void set##name2(type2 val) { \
    field2 = (type2)(intptr_t(val) | 1LL); \
  } \

#define UNION_FIELD_ACCESSORS3(name1, type1, field1, name2, type2, field2, name3, type3, field3) \
  inline bool has##name1() const { \
    return field1 && !(intptr_t(field1) & 3LL); \
  } \
  inline bool has##name2() const { \
    return bool(intptr_t(field2) & 1LL); \
  } \
  inline bool has##name3() const { \
    return bool(intptr_t(field3) & 2LL); \
  } \
  inline type1 get##name1() const { \
    assert(has##name1()); \
    return field1; \
  } \
  inline type2 get##name2() const { \
    assert(has##name2()); \
    return (type2)(intptr_t(field2) & ~1LL); \
  } \
  inline type3 get##name3() const { \
    return (type3)(intptr_t(field3) & ~2LL); \
  } \
  inline void set##name1(type1 val) { \
    field1 = val; \
  } \
  inline void set##name2(type2 val) { \
    field2 = (type2)(intptr_t(val) | 1LL); \
  } \
  inline void set##name3(type3 val) { \
    field3 = (type3)(intptr_t(val) | 2LL); \
  }

  // Note that reordering these is likely to require changes to the
  // translator.
  UNION_FIELD_ACCESSORS2(This, ObjectData*, m_this, \
                         Class, Class*, m_cls)
  static const int8_t kInvNameBit   = 0x1;
  static const int8_t kExtraArgsBit = 0x2;
  UNION_FIELD_ACCESSORS3(VarEnv, VarEnv*, m_varEnv, \
                         InvName, StringData*, m_invName, \
                         ExtraArgs, ExtraArgs*, m_extraArgs)

#undef UNION_FIELD_ACCESSORS2
#undef UNION_FIELD_ACCESSORS3

  // Accessors for extra arg queries.
  TypedValue* getExtraArg(unsigned ind) const {
    assert(hasExtraArgs() || hasVarEnv());
    return hasExtraArgs() ? getExtraArgs()->getExtraArg(ind) :
           hasVarEnv()    ? getVarEnv()->getExtraArg(ind) :
           static_cast<TypedValue*>(0);
  }
};

inline int32_t arOffset(const ActRec* ar, const ActRec* other) {
  return (intptr_t(other) - intptr_t(ar)) / sizeof(TypedValue);
}

inline ActRec* arAtOffset(const ActRec* ar, int32_t offset) {
  return (ActRec*)(intptr_t(ar) + intptr_t(offset * sizeof(TypedValue)));
}

inline ActRec* arFromSpOffset(const ActRec *sp, int32_t offset) {
  return arAtOffset(sp, offset);
}

inline void arSetSfp(ActRec* ar, const ActRec* sfp) {
  static_assert(offsetof(ActRec, m_savedRbp) == 0,
                "m_savedRbp should be at offset 0 of ActRec");
  static_assert(sizeof(ActRec*) <= sizeof(uint64_t),
                "ActRec* must be <= 64 bits");
  ar->m_savedRbp = (uint64_t)sfp;
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

private:
  void toStringFrame(std::ostream& os, const ActRec* fp,
                     int offset, const TypedValue* ftop,
                     const std::string& prefix) const;

public:
  static const int sSurprisePageSize;
  static const uint sMinStackElms;
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

  static inline size_t topOfStackOffset() {
    Stack *that = 0;
    return (size_t)&that->m_top;
  }

  static TypedValue* frameStackBase(const ActRec* fp);
  static TypedValue* generatorStackBase(const ActRec* fp);

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
 * handling FPI regions and generators correctly, and stopping when we
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
    fp->m_func->isGenerator() ? Stack::generatorStackBase(fp)
                              : Stack::frameStackBase(fp);
  MaybeConstTVPtr cursor = stackTop;
  assert(cursor <= base);

  if (auto fe = fp->m_func->findFPI(bcOffset)) {
    for (;;) {
      ActRec* ar;
      if (!fp->m_func->isGenerator()) {
        ar = arAtOffset(fp, -fe->m_fpOff);
      } else {
        // fp is pointing into the continuation object. Since fpOff is
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
