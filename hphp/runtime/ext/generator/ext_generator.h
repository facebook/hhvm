/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

class BaseGenerator {
public:
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
    assert(func->isGenerator());
    auto base = func->base();

    auto pc = func->unit()->at(base);
    auto DEBUG_ONLY op1 = decode_op(pc);
    auto DEBUG_ONLY op2 = decode_op(pc);
    assert(op1 == OpCreateCont);
    assert(op2 == OpPopC);

    return func->unit()->offsetOf(pc);
  }

  static size_t genSize(size_t ndSize, size_t frameSz) {
    return alignTypedValue(sizeof(NativeNode) + frameSz + ndSize) +
      sizeof(ObjectData);
  }

  static ObjectData* Alloc(Class* cls, size_t totalSize) {
    auto const node = reinterpret_cast<NativeNode*>(MM().objMalloc(totalSize));
    const size_t objOff = totalSize - sizeof(ObjectData);
    node->obj_offset = objOff;
    node->hdr.kind = HeaderKind::NativeData;
    auto const obj = new (reinterpret_cast<char*>(node) + objOff)
                     ObjectData(cls, ObjectData::HasNativeData);
    assert(obj->hasExactlyOneRef());
    assert(obj->noDestruct());
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
class Generator final : public BaseGenerator {
public:
  explicit Generator();
  ~Generator();
  Generator& operator=(const Generator& other);

  template <bool clone>
  static ObjectData* Create(const ActRec* fp, size_t numSlots,
                            jit::TCA resumeAddr, Offset resumeOffset);
  static Class* getClass() {
    assert(s_class);
    return s_class;
  }
  static constexpr ptrdiff_t objectOff() {
    return -(Native::dataOffset<Generator>());
  }
  static Generator* fromObject(ObjectData *obj) {
    return Native::data<Generator>(obj);
  }
  static ObjectData* allocClone(ObjectData *obj) {
    auto const genDataSz = Native::getNativeNode(
                             obj, getClass()->getNativeDataInfo())->obj_offset;
    auto const clone = BaseGenerator::Alloc(getClass(),
                         genDataSz + sizeof(ObjectData));
    UNUSED auto const genData = new (Native::data<Generator>(clone))
                                Generator();
    return clone;
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

template <bool clone>
ObjectData* Generator::Create(const ActRec* fp, size_t numSlots,
                              jit::TCA resumeAddr, Offset resumeOffset) {
  assert(fp);
  assert(fp->resumed() == clone);
  assert(fp->func()->isNonAsyncGenerator());
  const size_t frameSz = Resumable::getFrameSize(numSlots);
  const size_t genSz = genSize(sizeof(Generator), frameSz);
  auto const obj = BaseGenerator::Alloc(s_class, genSz);
  auto const genData = new (Native::data<Generator>(obj)) Generator();
  genData->resumable()->initialize<clone>(fp,
                                          resumeAddr,
                                          resumeOffset,
                                          frameSz,
                                          genSz);
  genData->setState(State::Created);
  return obj;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_GENERATOR_H_
