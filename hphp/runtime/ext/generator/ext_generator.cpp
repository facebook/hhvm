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

#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/spl/ext_spl.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/base/stats.h"

namespace HPHP {

Class* Generator::s_class = nullptr;
const StaticString Generator::s_className("Generator");

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

  assert(getState() != State::Running);
  tvRefcountedDecRef(m_key);
  tvRefcountedDecRef(m_value);

  // Free locals, but don't trigger the EventHook for FunctionReturn since
  // the generator has already been exited. We don't want redundant calls.
  ActRec* ar = actRec();
  frame_free_locals_inl_no_hook<false>(ar, ar->func()->numLocals());
}

Generator& Generator::operator=(const Generator& other) {
  auto const fp = other.actRec();
  const size_t numSlots = fp->func()->numSlotsInFrame();
  const size_t frameSz = Resumable::getFrameSize(numSlots);
  const size_t genSz = genSize(sizeof(Generator), frameSz);
  resumable()->initialize<true>(fp,
                                other.resumable()->resumeAddr(),
                                other.resumable()->resumeOffset(),
                                frameSz,
                                genSz);
  copyVars(fp);
  setState(other.getState());
  m_index = other.m_index;
  cellSet(other.m_key, m_key);
  cellSet(other.m_value, m_value);
  return *this;
}

void Generator::copyVars(const ActRec* srcFp) {
  const auto dstFp = actRec();
  const auto func = dstFp->func();
  assert(srcFp->func() == dstFp->func());

  for (Id i = 0; i < func->numLocals(); ++i) {
    tvDupFlattenVars(frame_local(srcFp, i), frame_local(dstFp, i));
  }

  if (dstFp->hasThis()) {
    dstFp->getThis()->incRefCount();
  }

  if (LIKELY(!(srcFp->func()->attrs() & AttrMayUseVV))) return;
  if (LIKELY(srcFp->m_varEnv == nullptr)) return;

  if (srcFp->hasExtraArgs()) {
    dstFp->setExtraArgs(srcFp->getExtraArgs()->clone(dstFp));
  } else {
    assert(srcFp->hasVarEnv());
    dstFp->setVarEnv(srcFp->getVarEnv()->clone(dstFp));
  }
}

void Generator::yield(Offset resumeOffset,
                      const Cell* key, const Cell value) {
  assert(getState() == State::Running);
  resumable()->setResumeAddr(nullptr, resumeOffset);

  if (key) {
    cellSet(*key, m_key);
    tvRefcountedDecRefNZ(*key);
    if (m_key.m_type == KindOfInt64) {
      int64_t new_index = m_key.m_data.num;
      m_index = new_index > m_index ? new_index : m_index;
    }
  } else {
    cellSet(make_tv<KindOfInt64>(++m_index), m_key);
  }
  cellSet(value, m_value);
  tvRefcountedDecRefNZ(value);

  setState(State::Started);
}

void Generator::done() {
  assert(getState() == State::Running);
  cellSetNull(m_key);
  cellSetNull(m_value);
  setState(State::Done);
}

const StaticString s__closure_("{closure}");
String HHVM_METHOD(Generator, getOrigFuncName) {
  Generator* gen = Native::data<Generator>(this_);
  const Func* origFunc = gen->actRec()->func();
  auto const origName = origFunc->isClosureBody() ? s__closure_.get()
                                                  : origFunc->name();
  assert(origName->isStatic());
  return String(const_cast<StringData*>(origName));
}

String HHVM_METHOD(Generator, getCalledClass) {
  Generator* gen = Native::data<Generator>(this_);
  String called_class;

  if (gen->actRec()->hasThis()) {
    called_class =
      gen->actRec()->getThis()->getVMClass()->name()->data();
  } else if (gen->actRec()->hasClass()) {
    called_class = gen->actRec()->getClass()->name()->data();
  } else {
    called_class = empty_string();
  }

  return called_class;
}

///////////////////////////////////////////////////////////////////////////////

class GeneratorExtension final : public Extension {
public:
  GeneratorExtension() : Extension("generator") {}

  void moduleInit() override {
    HHVM_ME(Generator, getOrigFuncName);
    HHVM_ME(Generator, getCalledClass);
    Native::registerNativeDataInfo<Generator>(
      Generator::s_className.get(),
      Native::NDIFlags::NO_SWEEP);
    loadSystemlib("generator");
    Generator::s_class = Unit::lookupClass(Generator::s_className.get());
    assert(Generator::s_class);
  }
};

static GeneratorExtension s_generator_extension;

///////////////////////////////////////////////////////////////////////////////
}
