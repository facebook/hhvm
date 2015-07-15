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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
class c_AsyncGeneratorWaitHandle;
class c_StaticWaitHandle;
class c_WaitableWaitHandle;

///////////////////////////////////////////////////////////////////////////////
// class AsyncGenerator

class AsyncGenerator final : public BaseGenerator {
public:
   AsyncGenerator() : m_waitHandle(nullptr) {}
  ~AsyncGenerator();

  static ObjectData* Create(const ActRec* fp, size_t numSlots,
                            jit::TCA resumeAddr, Offset resumeOffset);
  static Class* getClass() {
    assert(s_class);
    return s_class;
  }
  static constexpr ptrdiff_t objectOff() {
    return -(Native::dataOffset<AsyncGenerator>());
  }
  static AsyncGenerator* fromObject(ObjectData *obj) {
    return Native::data<AsyncGenerator>(obj);
  }

  c_AsyncGeneratorWaitHandle* await(Offset resumeOffset,
                                    c_WaitableWaitHandle* child);
  c_StaticWaitHandle* yield(Offset resumeOffset,
                            const Cell* key, Cell value);
  c_StaticWaitHandle* ret();
  c_StaticWaitHandle* fail(ObjectData* exception);
  void failCpp();

  ObjectData* toObject() {
    return Native::object<AsyncGenerator>(this);
  }

  bool isEagerlyExecuted() const {
    assert(getState() == State::Running);
    return m_waitHandle == nullptr;
  }

  c_AsyncGeneratorWaitHandle* getWaitHandle() const {
    assert(getState() == State::Running);
    return m_waitHandle;
  }

private:
  // valid only in Running state; null during eager execution
  c_AsyncGeneratorWaitHandle* m_waitHandle;

public:
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_H_
