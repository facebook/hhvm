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

#ifndef incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_H_
#define incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class AsyncGenerator

FORWARD_DECLARE_CLASS(AsyncGenerator);
FORWARD_DECLARE_CLASS(AsyncGeneratorWaitHandle);
FORWARD_DECLARE_CLASS(StaticWaitHandle);
FORWARD_DECLARE_CLASS(WaitableWaitHandle);
class c_AsyncGenerator : public BaseGenerator {
public:
  DECLARE_CLASS_NO_SWEEP(AsyncGenerator)

  explicit c_AsyncGenerator(Class* cls = c_AsyncGenerator::classof())
    : BaseGenerator(cls)
  {}
  ~c_AsyncGenerator();
  void t___construct();
  void t_next();
  void t_send(const Variant& value);
  void t_raise(const Object& exception);

public:
  static c_AsyncGenerator* Create(const ActRec* fp,
                                  size_t numSlots,
                                  jit::TCA resumeAddr,
                                  Offset resumeOffset);

  bool isEagerlyExecuted() const {
    assert(getState() == State::Running);
    return m_waitHandle == nullptr;
  }
  c_AsyncGeneratorWaitHandle* getWaitHandle() const {
    assert(getState() == State::Running);
    return m_waitHandle;
  }

  c_AsyncGeneratorWaitHandle* await(Offset resumeOffset,
                                    c_WaitableWaitHandle* child);
  c_StaticWaitHandle* yield(Offset resumeOffset,
                            const Cell* key, const Cell& value);
  c_StaticWaitHandle* ret();
  c_StaticWaitHandle* fail(ObjectData* exception);
  void failCpp();

private:
  // valid only in Running state; null during eager execution
  c_AsyncGeneratorWaitHandle* m_waitHandle;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_H_
