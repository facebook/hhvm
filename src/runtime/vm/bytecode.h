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

#ifndef incl_VM_BYTECODE_H_
#define incl_VM_BYTECODE_H_

#include "util/util.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/class_info.h"
#include "runtime/base/array/array_iterator.h"

#include "runtime/ext/bcmath/bcmath.h" // for MAX(a,b)

// We want to use the TypedValue macros inside this header file, but we don't
// want to pollute the environment of files that include this header file.
// Thus, we record whether they were already defined here. That way, we know
// whether we need to undefine the macros at the end of this header file.
#ifndef __HPHP_TV_MACROS__
#include "runtime/base/tv_macros.h"
#define __VM_BYTECODE_H_SHOULD_UNDEF_TV__
#endif

#include "runtime/vm/core_types.h"
#include "runtime/vm/class.h"
#include "runtime/vm/instance.h"
#include "runtime/vm/unit.h"

namespace HPHP {

namespace VM {

#define LITSTR_DECREF(s) do {                                                 \
  if ((s)->decRefCount() == 0) {                                              \
    (s)->release();                                                           \
  }                                                                           \
} while (0)

// SETOP_BODY() would ideally be an inline function, but the header
// dependencies for concat_assign() make this unfeasible.
#define SETOP_BODY(lhs, op, rhs) do {                                         \
  switch (op) {                                                               \
  case SetOpPlusEqual: tvAsVariant(lhs) += tvCellAsCVarRef(rhs); break;       \
  case SetOpMinusEqual: tvAsVariant(lhs) -= tvCellAsCVarRef(rhs); break;      \
  case SetOpMulEqual: tvAsVariant(lhs) *= tvCellAsCVarRef(rhs); break;        \
  case SetOpDivEqual: tvAsVariant(lhs) /= tvCellAsCVarRef(rhs); break;        \
  case SetOpConcatEqual: {                                                    \
    concat_assign(tvAsVariant(lhs), tvCellAsCVarRef(rhs));                    \
    break;                                                                    \
  }                                                                           \
  case SetOpModEqual: tvAsVariant(lhs) %= tvCellAsCVarRef(rhs); break;        \
  case SetOpAndEqual: tvAsVariant(lhs) &= tvCellAsCVarRef(rhs); break;        \
  case SetOpOrEqual: tvAsVariant(lhs) |= tvCellAsCVarRef(rhs); break;         \
  case SetOpXorEqual: tvAsVariant(lhs) ^= tvCellAsCVarRef(rhs); break;        \
  case SetOpSlEqual: tvAsVariant(lhs) <<= tvCellAsCVarRef(rhs); break;        \
  case SetOpSrEqual: tvAsVariant(lhs) >>= tvCellAsCVarRef(rhs); break;        \
  default: ASSERT(false);                                                     \
  }                                                                           \
} while (0)

inline void IncDecBody(unsigned char op, TypedValue* fr, TypedValue* to) {
  switch ((IncDecOp)op) {
  case PreInc: {
    if (fr->m_type == KindOfInt64) {
      ++(fr->m_data.num);
      tvDupCell(fr, to);
    } else {
      ++(tvAsVariant(fr));
      tvReadCell(fr, to);
    }
    break;
  }
  case PostInc: {
    if (fr->m_type == KindOfInt64) {
      tvDupCell(fr, to);
      ++(fr->m_data.num);
    } else {
      tvReadCell(fr, to);
      ++(tvAsVariant(fr));
    }
    break;
  }
  case PreDec: {
    if (fr->m_type == KindOfInt64) {
      --(fr->m_data.num);
      tvDupCell(fr, to);
    } else {
      --(tvAsVariant(fr));
      tvReadCell(fr, to);
    }
    break;
  }
  case PostDec: {
    if (fr->m_type == KindOfInt64) {
      tvDupCell(fr, to);
      --(fr->m_data.num);
    } else {
      tvReadCell(fr, to);
      --(tvAsVariant(fr));
    }
    break;
  }
  default: ASSERT(false);
  }
}

// Call activation record.  The order assumes that stacks grow toward lower
// addresses.
class Func;
struct ActRec {
  // This pair of uint64_t's must be the first two elements in the structure
  // so that the pointer to the ActRec can also be used for RBP chaining. Note
  // That ActRec's are also x64 frames, so this is an implicit machine
  // dependency.
  union {
    TypedValue _dummyA;
    struct {
      uint64_t m_savedRbp;   // Previous hardware frame pointer/ActRec.
      uint64_t m_savedRip;   // In-TC address to return to.
    };
  };
  union {
    TypedValue _dummyB;
    struct {
      const Func* m_func;    // Function.
      union {
        ObjectData* m_this;  // This.
        Class* m_cls;        // Late bound class.
      };
    };
  };
  union {
    TypedValue _dummyC;
    struct {
      union {
        VarEnv* m_varEnv;   // Variable environment.
        StringData* m_invName; // Invoked function name (used for __call).
      };
      uint32_t m_soff;        // Saved offset from beginning of Func's bytecode.
      int32 m_numArgs;        // Number of arguments passed.
    };
  };
  union {
    struct {
      TypedValue m_r;         // Return value splatted here by interpreter.
    };
    struct {
      // An array containing the "scope" of static locals in this frame. This is
      // necessary because of some strange rules around identity of static
      // locals. In non-private methods, each class context gets its own copy of
      // static locals. This class context is *not* the lexical class context,
      // and is distinct from the late-bound class in that it doesn't propagate
      // across self:: and parent:: calls. In closures, each closure
      // instantiation gets its own copy. In generators created from
      // closures, each generator gets its own copy.
      //
      // If it's known that the function doesn't contain a static local OR (is
      // not a non-private method AND is not a closure AND is not a generator
      // from a closure), it's safe to leave this uninitialized.
      HphpArray* m_staticLocalCtx;
    };
  };

  // To conserve space, we use unions for a pairs of mutually exclusive
  // fields (fields that are not used at the same time). We use unions for
  // m_this/m_cls and m_varEnv/m_invName.
  //
  // The least significant bit is used as a marker for each pair of fields
  // so that we can distinguish at runtime which field is valid. We define
  // accessors below to encapsulate this logic.
  //
  // Note that m_invName is only used in between FPush and FCall. Thus once
  // is activated, it is safe to directly access m_varEnv without using
  // accessors.

#define UNION_FIELD_ACCESSORS(name1, type1, field1, name2, type2, field2) \
  inline bool has##name1() const { \
    return field1 && !(intptr_t(field1) & 1LL); \
  } \
  inline bool has##name2() const { \
    return (intptr_t(field2) & 1LL); \
  } \
  inline type1 get##name1() const { \
    ASSERT(has##name1()); \
    return field1; \
  } \
  inline type2 get##name2() const { \
    ASSERT(has##name2()); \
    return (type2)(intptr_t(field2) & ~1LL); \
  } \
  inline void set##name1(type1 val) { \
    field1 = val; \
  } \
  inline void set##name2(type2 val) { \
    field2 = (type2)(intptr_t(val) | 1LL); \
  }

  UNION_FIELD_ACCESSORS(This, ObjectData*, m_this, Class, Class*, m_cls)
  UNION_FIELD_ACCESSORS(VarEnv, VarEnv*, m_varEnv, InvName, StringData*,
                        m_invName)

#undef UNION_FIELD_ACCESSORS
};

inline int32 arOffset(const ActRec* ar, const ActRec* other) {
  return (intptr_t(other) - intptr_t(ar)) / sizeof(TypedValue);
}

inline ActRec* arAtOffset(const ActRec* ar, int32 offset) {
  return (ActRec*)(intptr_t(ar) + intptr_t(offset * sizeof(TypedValue)));
}

inline ActRec* arFromSpOffset(const ActRec *sp, int32 offset) {
  return arAtOffset(sp, offset);
}

inline void arSetSfp(ActRec* ar, const ActRec* sfp) {
  CT_ASSERT(offsetof(ActRec, m_savedRbp) == 0);
  ar->m_savedRbp = (uint64_t)sfp;
}

PreClass* arGetContextPreClass(const ActRec* ar);

// Used by extension functions that take a PHP "callback", since they need to
// figure out the callback context once and call it multiple times. (e.g.
// array_map, array_filter, ...)
struct CallCtx {
  const Func* func;
  ObjectData* this_;
  Class* cls;
  StringData* invName;
};

struct MIterCtx {
  TypedValue m_key;
  TypedValue m_val;
  MutableArrayIter *m_mArray; // big! Defer allocation.
  MIterCtx(ArrayData *ad) {
    ASSERT(!ad->isStatic());
    tvWriteUninit(&m_key);
    tvWriteUninit(&m_val);
    m_mArray = new MutableArrayIter(ad, &tvAsVariant(&m_key),
                                    tvAsVariant(&m_val));
  }
  MIterCtx(const Variant* var) {
    tvWriteUninit(&m_key);
    tvWriteUninit(&m_val);
    m_mArray = new MutableArrayIter(var, &tvAsVariant(&m_key),
                                    tvAsVariant(&m_val));
  }
  ~MIterCtx() {
    delete m_mArray;
  }
};

struct Iter {
  enum Type {
    TypeUndefined,
    TypeArray,
    TypeMutableArray,
    TypeIterator  // for "implements Iterator" objects
  };
 private:
  // C++ won't let you have union members with constructors. So we get to
  // implement unions by hand.
  char m_u[MAX(sizeof(ArrayIter), sizeof(MIterCtx))];
 public:
  int32_t _pad;
  Type m_itype;
  MIterCtx& marr() {
    return *(MIterCtx*)m_u;
  }
  ArrayIter& arr() {
    return *(ArrayIter*)m_u;
  }
} __attribute__ ((aligned(16)));

static const size_t kNumIterCells = sizeof(Iter) / sizeof(Cell);
static const size_t kNumActRecCells = sizeof(ActRec) / sizeof(Cell);

struct Fault {
  enum FaultType {
    KindOfThrow,
    KindOfExit,
    KindOfFatal,
    KindOfCPPException
  };
  FaultType m_faultType;
  TypedValue m_userException;
  Exception *m_cppException;
};

enum UnwindStatus {
  UnwindResumeVM,
  UnwindNextFrame,
  UnwindPropagate,
  UnwindIgnore,
  UnwindExit,
};

// Interpreter evaluation stack.
class Stack {
private:
  TypedValue* m_elms;
  TypedValue* m_top;
  TypedValue* m_base; // Stack grows down, so m_base is beyond the end of
                      // m_elms.
  size_t      m_maxElms;

public:
  void* getStackLowAddress() { return m_elms; }
  void* getStackHighAddress() { return m_base; }
  void toStringElm(std::ostream& os, TypedValue* vv) const;
  void toStringIter(std::ostream& os, Iter* it) const;
  void clearEvalStack(ActRec* fp, int32 numArgs);
private:
  void toStringFrag(std::ostream& os, const ActRec* fp,
                    const TypedValue* top) const;
  void toStringAR(std::ostream& os, const ActRec* fp,
                  const FPIEnt *fe, const TypedValue* top) const;
  void toStringFragAR(std::ostream& os, const ActRec* fp,
                    int offset, const TypedValue* top) const;
  void toStringFrame(std::ostream& os, const ActRec* fp,
                     int offset, const TypedValue* ftop,
                     const std::string& prefix) const;

  UnwindStatus unwindFrag(ActRec* fp, int offset, PC& pc, Fault& f);
  void unwindARFrag(ActRec* ar);
  void unwindAR(ActRec* fp, int offset, const FPIEnt* fe);
public:
  // Note: maxelms must be a power of two
  static const int kDefaultStackMax = 0x4000U; // Must be power of two.
  Stack(size_t maxelms = kDefaultStackMax);
  ~Stack();

  std::string toString(const ActRec* fp, int offset,
                       std::string prefix="") const;

  UnwindStatus unwindFrame(ActRec*& fp, int offset, PC& pc, Fault& f);
  void clear();

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

  inline size_t ALWAYS_INLINE count() const {
    return ((uintptr_t)m_base - (uintptr_t)m_top) / sizeof(TypedValue);
  }

  // Same as discard(), but meant to replace popC() iff the interpreter knows
  // for certain that decrementing a refcount is unnecessary.
  inline void ALWAYS_INLINE popX() {
    ASSERT(m_top != m_base);
    ASSERT(!IS_REFCOUNTED_TYPE(m_top->m_type));
    m_top++;
  }

  inline void ALWAYS_INLINE popC() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type != KindOfVariant);
    tvRefcountedDecRefCell(m_top);
    m_top++;
  }

  inline void ALWAYS_INLINE popV() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type == KindOfVariant);
    ASSERT(m_top->m_data.ptv != NULL);
    tvDecRefVar(m_top);
    m_top++;
  }

  inline void ALWAYS_INLINE popH() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type == KindOfHome);
    ASSERT(m_top->m_data.ptv != NULL);
    m_top++;
  }

  template <bool canThrow>
  inline void ALWAYS_INLINE popTVImpl() {
    ASSERT(m_top != m_base);
    tvRefcountedDecRefImpl<canThrow>(m_top);
    m_top++;
  }

#define popTV()    popTVImpl<true>()

  inline void ALWAYS_INLINE popI() {
    ASSERT(((Iter*)m_top)->m_itype == Iter::TypeUndefined);
    m_top += sizeof(Iter) / sizeof(TypedValue);
    ASSERT((uintptr_t)m_top <= (uintptr_t)m_base);
  }

  template <bool canThrow>
  inline void ALWAYS_INLINE popARImpl() {
    ASSERT(m_top != m_base);
    ActRec* ar = (ActRec*)m_top;
    if (ar->hasThis()) {
      ObjectData* this_ = ar->getThis();
      if (this_->decRefCount() == 0) {
        this_->releaseImpl<canThrow>();
      }
    }
    if (ar->hasInvName()) {
      StringData* invName = ar->getInvName();
      if (invName->decRefCount() == 0) {
        invName->release();
      }
    }
    m_top += kNumActRecCells;
    ASSERT((uintptr_t)m_top <= (uintptr_t)m_base);
  }

#define popAR()    popARImpl<true>()

  inline void ALWAYS_INLINE discardAR() {
    ASSERT(m_top != m_base);
    m_top += kNumActRecCells;
    ASSERT((uintptr_t)m_top <= (uintptr_t)m_base);
  }

  inline void ALWAYS_INLINE ret() {
    // Leave part of the activation on the stack, since the return value now
    // resides there.
    m_top += kNumActRecCells - 1;
    ASSERT((uintptr_t)m_top <= (uintptr_t)m_base);
  }

  inline void ALWAYS_INLINE discard() {
    ASSERT(m_top != m_base);
    m_top++;
  }

  inline void ALWAYS_INLINE ndiscard(size_t n) {
    ASSERT((uintptr_t)&m_top[n] <= (uintptr_t)m_base);
    m_top += n;
  }

  inline void ALWAYS_INLINE dup() {
    ASSERT(m_top != m_base);
    ASSERT(m_top != m_elms);
    ASSERT(m_top->m_type != KindOfVariant);
    Cell* fr = (Cell*)m_top;
    m_top--;
    Cell* to = (Cell*)m_top;
    TV_DUP_CELL(fr, to);
  }

  inline void ALWAYS_INLINE box() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type != KindOfVariant);
    tvBox(m_top);
  }

  inline void ALWAYS_INLINE unbox() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type == KindOfVariant);
    TV_UNBOX(m_top);
  }

  inline void ALWAYS_INLINE pushUninit() {
    ASSERT(m_top != m_elms);
    m_top--;
    TV_WRITE_UNINIT(m_top);
  }

  inline void ALWAYS_INLINE pushNull() {
    ASSERT(m_top != m_elms);
    m_top--;
    TV_WRITE_NULL(m_top);
  }

  #define PUSH_METHOD(name, type, field, value)                               \
  inline void ALWAYS_INLINE push##name() {                                    \
    ASSERT(m_top != m_elms);                                                  \
    m_top--;                                                                  \
    m_top->m_data.field = value;                                              \
    m_top->_count = 0;                                                        \
    m_top->m_type = type;                                                     \
  }
  PUSH_METHOD(True, KindOfBoolean, num, 1)
  PUSH_METHOD(False, KindOfBoolean, num, 0)

  #define PUSH_METHOD_ARG(name, type, field, argtype, arg)                    \
  inline void ALWAYS_INLINE push##name(argtype arg) {                         \
    ASSERT(m_top != m_elms);                                                  \
    m_top--;                                                                  \
    m_top->m_data.field = arg;                                                \
    m_top->_count = 0;                                                        \
    m_top->m_type = type;                                                     \
  }
  PUSH_METHOD_ARG(Int, KindOfInt64, num, int64, i)
  PUSH_METHOD_ARG(Double, KindOfDouble, dbl, double, d)

  // This should only be called directly when the caller has
  // already adjusted the refcount appropriately
  inline void ALWAYS_INLINE pushStringNoRc(StringData* s) {
    ASSERT(m_top != m_elms);
    m_top--;
    m_top->m_data.pstr = s;
    m_top->_count = 0;
    m_top->m_type = KindOfString;
  }

  inline void ALWAYS_INLINE pushString(StringData* s) {
    pushStringNoRc(s);
    s->incRefCount();
  }

  inline void ALWAYS_INLINE pushStaticString(StringData* s) {
    ASSERT(s->isStatic()); // No need to call s->incRefCount().
    pushStringNoRc(s);
  }

  // This should only be called directly when the caller has
  // already adjusted the refcount appropriately
  inline void ALWAYS_INLINE pushArrayNoRc(ArrayData* a) {
    ASSERT(m_top != m_elms);
    m_top--;
    m_top->m_data.parr = a;
    m_top->_count = 0;
    m_top->m_type = KindOfArray;
  }

  inline void ALWAYS_INLINE pushArray(ArrayData* a) {
    pushArrayNoRc(a);
    a->incRefCount();
  }

  inline void ALWAYS_INLINE pushStaticArray(ArrayData* a) {
    ASSERT(a->isStatic()); // No need to call a->incRefCount().
    pushArrayNoRc(a);
  }

  // This should only be called directly when the caller has
  // already adjusted the refcount appropriately
  inline void ALWAYS_INLINE pushObjectNoRc(ObjectData* o) {
    ASSERT(m_top != m_elms);
    m_top--;
    m_top->m_data.pobj = o;
    m_top->_count = 0;
    m_top->m_type = KindOfObject;
  }

  inline void ALWAYS_INLINE pushObject(ObjectData* o) {
    pushObjectNoRc(o);
    o->incRefCount();
  }

  inline Cell* ALWAYS_INLINE allocC() {
    ASSERT(m_top != m_elms);
    m_top--;
    return (Cell*)m_top;
  }

  // XXX Unused.
  inline Var* ALWAYS_INLINE allocV() {
    ASSERT(m_top != m_elms);
    m_top--;
    return (Var*)m_top;
  }

  inline Home* ALWAYS_INLINE allocH() {
    ASSERT(m_top != m_elms);
    m_top--;
    return (Home*)m_top;
  }

  inline TypedValue* ALWAYS_INLINE allocTV() {
    ASSERT(m_top != m_elms);
    m_top--;
    return m_top;
  }

  inline ActRec* ALWAYS_INLINE allocA() {
    ASSERT((uintptr_t)&m_top[-kNumActRecCells] >= (uintptr_t)m_elms);
    ASSERT(kNumActRecCells * sizeof(Cell) == sizeof(ActRec));
    m_top -= kNumActRecCells;
    return (ActRec*)m_top;
  }

  inline void ALWAYS_INLINE allocI() {
    ASSERT(kNumIterCells * sizeof(Cell) == sizeof(Iter));
    ASSERT((uintptr_t)&m_top[-kNumIterCells] >= (uintptr_t)m_elms);
    m_top -= kNumIterCells;
  }

  inline Cell* ALWAYS_INLINE topC() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type != KindOfVariant);
    return (Cell*)m_top;
  }

  inline Var* ALWAYS_INLINE topV() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type == KindOfVariant);
    return (Var*)m_top;
  }

  inline Home* ALWAYS_INLINE topH() {
    ASSERT(m_top != m_base);
    ASSERT(m_top->m_type == KindOfHome);
    return (Home*)m_top;
  }

  inline TypedValue* ALWAYS_INLINE topTV() {
    ASSERT(m_top != m_base);
    return m_top;
  }

  inline Cell* ALWAYS_INLINE indC(size_t ind) {
    ASSERT(m_top != m_base);
    ASSERT(m_top[ind].m_type != KindOfVariant);
    return (Cell*)(&m_top[ind]);
  }

  inline Var* ALWAYS_INLINE indV(size_t ind) {
    ASSERT(m_top != m_base);
    ASSERT(m_top[ind].m_type == KindOfVariant);
    return (Var*)(&m_top[ind]);
  }

  inline Home* ALWAYS_INLINE indH(size_t ind) {
    ASSERT(m_top != m_base);
    ASSERT(m_top[ind].m_type == KindOfHome);
    return (Home*)(&m_top[ind]);
  }

  inline TypedValue* ALWAYS_INLINE indTV(size_t ind) {
    ASSERT(m_top != m_base);
    return &m_top[ind];
  }
};

///////////////////////////////////////////////////////////////////////////////
}
}

// Undefine the TypedValue macros if appropriate
#ifdef __VM_BYTECODE_H_SHOULD_UNDEF_TV__
#undef __VM_BYTECODE_H_SHOULD_UNDEF_TV__
#include <runtime/base/undef_tv_macros.h>
#endif

#endif // __VM_BYTECODE_H__
