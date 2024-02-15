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

#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/base/stats.h"

namespace HPHP {

Generator::Generator()
  : m_index(-1LL)
  , m_key(make_tv<KindOfInt64>(-1LL))
  , m_value(make_tv<KindOfNull>())
{
}

Generator::~Generator() {
  if (LIKELY(getState() == State::Done)) {
    return;
  }

  assertx(getState() != State::Running);
  tvDecRefGen(m_key);
  tvDecRefGen(m_value);

  // Free locals, but don't trigger the EventHook for FunctionReturn since
  // the generator has already been exited. We don't want redundant calls.
  ActRec* ar = actRec();
  frame_free_locals_inl_no_hook(ar, ar->func()->numLocals());
}

Generator& Generator::operator=(const Generator& other) {
  auto const fp = other.actRec();
  const size_t numSlots = fp->func()->numSlotsInFrame();
  const size_t frameSz = Resumable::getFrameSize(numSlots);
  const size_t genSz = genSize(sizeof(Generator), frameSz);
  const size_t arOff = frameSz + sizeof(NativeNode);
  auto node = reinterpret_cast<NativeNode*>(
    reinterpret_cast<char*>(this) - arOff
  );
  node->arOff() = arOff;
  resumable()->initialize<true>(fp,
                                other.resumable()->resumeAddr(),
                                other.resumable()->suspendOffset(),
                                frameSz,
                                genSz);
  copyVars(fp);
  setState(other.getState());
  m_index = other.m_index;
  tvSet(other.m_key, m_key);
  tvSet(other.m_value, m_value);
  return *this;
}

ObjectData* Generator::Create(const ActRec* fp, size_t numSlots,
                              jit::TCA resumeAddr, Offset suspendOffset) {
  assertx(fp);
  assertx(!isResumed(fp));
  assertx(fp->func()->isNonAsyncGenerator());
  const size_t frameSz = Resumable::getFrameSize(numSlots);
  const size_t genSz = genSize(sizeof(Generator), frameSz);
  auto const obj = BaseGenerator::Alloc<Generator>(classof(), genSz);
  auto const genData = new (Native::data<Generator>(obj)) Generator();
  genData->resumable()->initialize<false>(fp,
                                          resumeAddr,
                                          suspendOffset,
                                          frameSz,
                                          genSz);
  genData->setState(State::Created);
  return obj;
}

void Generator::copyVars(const ActRec* srcFp) {
  const auto dstFp = actRec();
  const auto func = dstFp->func();
  assertx(srcFp->func() == dstFp->func());

  for (Id i = 0; i < func->numLocals(); ++i) {
    tvDup(*frame_local(srcFp, i), frame_local(dstFp, i));
  }

  if (func->cls() && dstFp->hasThis()) {
    dstFp->getThis()->incRefCount();
  }
}

void Generator::yield(Offset suspendOffset,
                      const TypedValue* key, const TypedValue value) {
  assertx(isRunning());
  resumable()->setResumeAddr(nullptr, suspendOffset);
  setState(State::Started);

  if (key) {
    tvSet(*key, m_key);
    tvDecRefGenNZ(*key);
    if (m_key.m_type == KindOfInt64) {
      int64_t new_index = m_key.m_data.num;
      m_index = new_index > m_index ? new_index : m_index;
    }
  } else {
    tvSet(make_tv<KindOfInt64>(++m_index), m_key);
  }
  tvSet(value, m_value);
  tvDecRefGenNZ(value);
}

void Generator::done(TypedValue tv) {
  assertx(isRunning());
  setState(State::Done);

  tvSetNull(m_key);
  tvSet(tv, m_value);
}

bool Generator::successfullyFinishedExecuting() {
  // `getReturn` needs to know whether a generator finished successfully or
  // whether an exception occurred during its execution. For every other use
  // case a failed generator was identical to one that finished executing, but
  // `getReturn` wants to throw an exception if the generator threw an
  // exception. Since we use the same variable to store the yield result and
  // the return value, and since we dont have a separate state to represent a
  // failed generator, we use an unintialized value to flag that the generator
  // failed (rather than NULL, which we use for a successful generator without
  // a return value).
  return getState() == State::Done &&
         m_value.m_type != KindOfUninit;
}

const StaticString s__closure_("{closure}");
String HHVM_METHOD(Generator, getOrigFuncName) {
  Generator* gen = Native::data<Generator>(this_);
  const Func* origFunc = gen->actRec()->func();
  auto const origName = origFunc->isClosureBody() ? s__closure_.get()
                                                  : origFunc->name();
  assertx(origName->isStatic());
  return String(const_cast<StringData*>(origName));
}

String HHVM_METHOD(Generator, getCalledClass) {
  auto const gen = Native::data<Generator>(this_);
  auto const ar = gen->actRec();

  if (ar->func()->cls()) {
    auto const cls = ar->hasThis() ?
      ar->getThis()->getVMClass() : ar->getClass();

    return cls->nameStr();
  }

  return empty_string();
}

///////////////////////////////////////////////////////////////////////////////

struct GeneratorExtension final : Extension {
  GeneratorExtension() : Extension("generator", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleRegisterNative() override {
    HHVM_ME(Generator, getOrigFuncName);
    HHVM_ME(Generator, getCalledClass);
    Native::registerNativeDataInfo<Generator>(
      Native::NDIFlags::NO_SWEEP | Native::NDIFlags::CTOR_THROWS);
  }
};

static GeneratorExtension s_generator_extension;

///////////////////////////////////////////////////////////////////////////////
}
