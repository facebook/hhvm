/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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


#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class BaseGeneratorData

class BaseGeneratorData {
public:
  enum class State : uint8_t {
    Created = 0,  // generator was created but never iterated
    Started = 1,  // generator was iterated but not currently running
    Running = 2,  // generator is currently being iterated
    Done    = 3   // generator has finished its execution
  };
  static constexpr ptrdiff_t resumableOff() {
    return -sizeof(Resumable);
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
    return offsetof(BaseGeneratorData, m_state);
  }

  Resumable* resumable() const {
    return reinterpret_cast<Resumable*>(
      const_cast<char*>(reinterpret_cast<const char*>(this) + resumableOff()));
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
    if (getState() == State::Running) {
      throw_exception(
        SystemLib::AllocExceptionObject("Generator is already running")
      );
    }
    if (getState() == State::Done) {
      throw_exception(
        SystemLib::AllocExceptionObject("Generator is already finished")
      );
    }
    assert(getState() == State::Created || getState() == State::Started);
    setState(State::Running);
  }

  /**
   * Get adjusted generator function base() where the real user code starts.
   *
   * Skips CreateCont and PopC opcodes.
   */
  static Offset userBase(const Func* func) {
    assert(func->isGenerator());
    auto base = func->base();

    DEBUG_ONLY auto op = reinterpret_cast<const Op*>(func->unit()->at(base));
    assert(op[0] == OpCreateCont);
    assert(op[1] == OpPopC);

    return base + 2;
  }

private:
  State m_state;
};

///////////////////////////////////////////////////////////////////////////////
// class GeneratorData
class GeneratorData final : public BaseGeneratorData {
public:
  ~GeneratorData();

  template <bool clone>
  static ObjectData* Create(const ActRec* fp, size_t numSlots,
                            jit::TCA resumeAddr, Offset resumeOffset);
  static GeneratorData* fromObject(ObjectData *obj);
  static GeneratorData* Clone(ObjectData* obj);
  static constexpr ptrdiff_t objectOff() {
    return sizeof(GeneratorData);
  }

  void yield(Offset resumeOffset, const Cell* key, Cell value);
  void copyVars(const ActRec *fp);
  void ret() { done(); }
  void fail() { done(); }
  ObjectData* toObject() {
    return reinterpret_cast<ObjectData*>(
      reinterpret_cast<char*>(this) + objectOff());
  }

private:
  explicit GeneratorData();
  void done();

public:
  int64_t m_index;
  Cell m_key;
  Cell m_value;
};
///////////////////////////////////////////////////////////////////////////////
// class BaseGenerator

class BaseGenerator : public
      ExtObjectDataFlags<ObjectData::HasClone> {
public:
  explicit BaseGenerator(Class* cls)
    : ExtObjectDataFlags(cls, HeaderKind::ResumableObj)
  {}
};

///////////////////////////////////////////////////////////////////////////////
// class Generator

class c_Generator final : public BaseGenerator {
public:
  DECLARE_CLASS_NO_SWEEP(Generator)

  explicit c_Generator(Class* cls = c_Generator::classof())
    : BaseGenerator(cls)
  {}
  ~c_Generator() {
    data()->~GeneratorData();
  }

  void t___construct();
  Variant t_current();
  Variant t_key();
  void t_next();
  void t_rewind();
  bool t_valid();
  void t_send(const Variant& v);
  void t_raise(const Variant& v);
  String t_getorigfuncname();
  String t_getcalledclass();

  GeneratorData *data() {
    return reinterpret_cast<GeneratorData*>(reinterpret_cast<char*>(
      static_cast<BaseGenerator*>(this)) - GeneratorData::objectOff());
  }
};

template <bool clone>
ObjectData* GeneratorData::Create(const ActRec* fp, size_t numSlots,
                                  jit::TCA resumeAddr, Offset resumeOffset) {
  assert(fp);
  assert(fp->resumed() == clone);
  assert(fp->func()->isNonAsyncGenerator());
  void* genDataPtr = Resumable::Create<clone,
                       sizeof(GeneratorData) + sizeof(c_Generator)>(
                       fp, numSlots, resumeAddr, resumeOffset);
  GeneratorData* genData = new (genDataPtr) GeneratorData();
  auto const gen = new (genData + 1) c_Generator();
  assert(gen->hasExactlyOneRef());
  assert(gen->noDestruct());
  genData->setState(State::Created);
  return static_cast<ObjectData*>(gen);
}

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/ext_generator_inl.h"

#endif // incl_HPHP_EXT_GENERATOR_H_
