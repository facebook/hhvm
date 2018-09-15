/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_GENERATOR_H_
#define incl_HPHP_EXT_GENERATOR_H_


#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class BaseGenerator

struct BaseGenerator {
  enum class State : uint8_t {
    Created = 0,  // generator was created but never iterated
    Started = 1,  // generator was iterated but not currently running
    Priming = 2,  // generator is advancing to the first yield
    Running = 3,  // generator is currently being iterated
    Done    = 4   // generator has finished its execution
  };

  static constexpr ptrdiff_t resumableOff() {
    return offsetof(BaseGenerator, m_resumable);
  }
  static constexpr ptrdiff_t arOff() {
    return resumableOff() + Resumable::arOff();
  }
  static constexpr ptrdiff_t resumeAddrOff() {
    return resumableOff() + Resumable::resumeAddrOff();
  }
  static constexpr ptrdiff_t resumeOffsetOff() {
    return resumableOff() + Resumable::resumeOffsetOff();
  }
  static constexpr ptrdiff_t stateOff() {
    return offsetof(BaseGenerator, m_state);
  }
  /**
   * Get adjusted Generator function base() where the real user code starts.
   *
   * Skips CreateCont and PopC opcodes.
   */
  static Offset userBase(const Func* func) {
    assertx(func->isGenerator());
    auto base = func->base();

    // Skip past VerifyParamType and EntryNoop bytecodes
    auto pc = func->unit()->at(base);
    auto past = func->unit()->at(func->past());
    while (peek_op(pc) != OpCreateCont) {
      pc += instrLen(pc);
      always_assert(pc < past);
    }

    auto DEBUG_ONLY op1 = decode_op(pc);
    auto DEBUG_ONLY op2 = decode_op(pc);
    assertx(op1 == OpCreateCont);
    assertx(op2 == OpPopC);

    return func->unit()->offsetOf(pc);
  }

  static size_t genSize(size_t ndSize, size_t frameSz) {
    return alignTypedValue(sizeof(NativeNode) + frameSz + ndSize) +
      sizeof(ObjectData);
  }

  template<class T>
  static ObjectData* Alloc(Class* cls, size_t totalSize) {
    auto const node = reinterpret_cast<NativeNode*>(
        tl_heap->objMalloc(totalSize)
    );
    auto const obj_offset = totalSize - sizeof(ObjectData);
    auto const objmem = reinterpret_cast<char*>(node) + obj_offset;
    auto const datamem = objmem - sizeof(T);
    auto const ar_off = (char*)((T*)datamem)->actRec() - (char*)node;
    auto const tyindex = type_scan::getIndexForMalloc<T>();
    node->obj_offset = obj_offset;
    node->initHeader_32_16(HeaderKind::NativeData, ar_off, tyindex);
    auto const obj = new (objmem) ObjectData(cls, 0, HeaderKind::NativeObject);
    assertx((void*)obj == (void*)objmem);
    assertx(obj->hasExactlyOneRef());
    assertx(obj->noDestruct());
    return obj;
  }

  Resumable* resumable() const {
    return const_cast<Resumable*>(&m_resumable);
  }

  ActRec* actRec() const {
    return resumable()->actRec();
  }

  State getState() const {
    return m_state;
  }

  void setState(State state) {
    m_state = state;
  }

  bool isRunning() const {
    return getState() == State::Priming || getState() == State::Running;
  }

  void startedCheck() {
    if (getState() == State::Created) {
      throw_exception(
        SystemLib::AllocExceptionObject("Need to call next() first")
      );
    }
  }

  void preNext(bool checkStarted) {
    if (checkStarted) {
      startedCheck();
    }
    switch (getState()) {
      case State::Created:
        setState(State::Priming);
        break;
      case State::Started:
        setState(State::Running);
        break;
      // For our purposes priming is basically running.
      case State::Priming:
      case State::Running:
        throw_exception(
          SystemLib::AllocExceptionObject("Generator is already running")
        );
        break;
      case State::Done:
        throw_exception(
          SystemLib::AllocExceptionObject("Generator is already finished")
        );
        break;
    }
  }

  Resumable m_resumable;
  State m_state;
};

// Resumable stores function locals and iterators in front of it
static_assert(offsetof(BaseGenerator, m_resumable) == 0,
              "Resumable must be the first member of the Generator");

///////////////////////////////////////////////////////////////////////////////
// class Generator
struct Generator final : BaseGenerator {
  explicit Generator();
  ~Generator();
  Generator& operator=(const Generator& other);

  static ObjectData* Create(const ActRec* fp, size_t numSlots,
                            jit::TCA resumeAddr, Offset resumeOffset);
  static Class* getClass() {
    assertx(s_class);
    return s_class;
  }
  static constexpr ptrdiff_t objectOff() {
    return -(Native::dataOffset<Generator>());
  }
  static Generator* fromObject(ObjectData *obj) {
    return Native::data<Generator>(obj);
  }

  void yield(Offset resumeOffset, const Cell* key, Cell value);
  void copyVars(const ActRec *fp);
  void ret(TypedValue tv) { done(tv); }
  void fail() { done(make_tv<KindOfUninit>()); }
  bool successfullyFinishedExecuting();
  ObjectData* toObject() {
    return Native::object<Generator>(this);
  }

private:
  void done(TypedValue tv);

public:
  int64_t m_index;
  Cell m_key;
  Cell m_value;
  TypedValue m_delegate;

  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_GENERATOR_H_
