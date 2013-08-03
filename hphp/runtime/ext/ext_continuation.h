/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_CONTINUATION_H_
#define incl_HPHP_EXT_CONTINUATION_H_


#include "hphp/runtime/base/base-includes.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS_BUILTIN(Continuation);
FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);
Object f_hphp_create_continuation(CStrRef clsname, CStrRef funcname, CStrRef origFuncName, CArrRef args = null_array);

///////////////////////////////////////////////////////////////////////////////
// class Continuation

class c_Continuation : public ExtObjectData {
 public:
  DECLARE_CLASS_NO_ALLOCATION(Continuation, Continuation, ObjectData)
  virtual void sweep();
  void operator delete(void* p) {
    c_Continuation* this_ = (c_Continuation*)p;
    DELETEOBJSZ(this_->getObjectSize())(this_);
  }

  explicit c_Continuation(Class* cls = c_Continuation::s_cls);
  ~c_Continuation();

public:
  static constexpr uint startedOffset() {
    return offsetof(c_Continuation, o_subclassData);
  }
  bool started() const { return o_subclassData.u8[0]; }
  void start() { o_subclassData.u8[0] = true; }

  enum ContState : uint8_t {
    Running = 1,
    Done    = 2
  };
  static constexpr uint stateOffset() {
    return offsetof(c_Continuation, o_subclassData) + 1;
  }
  bool done() const { return o_subclassData.u8[1] & ContState::Done; }
  void setDone() { o_subclassData.u8[1]  =  ContState::Done; }

  bool running() const { return o_subclassData.u8[1] & ContState::Running; }
  void setRunning() { o_subclassData.u8[1]  =  ContState::Running; }
  void setStopped() { o_subclassData.u8[1] &= ~ContState::Running; }

  void t___construct();
  void t_update(int64_t label, CVarRef value);
  void t_update_key(int64_t label, CVarRef value, CVarRef key);
  Object t_getwaithandle();
  int64_t t_getlabel();
  Variant t_current();
  Variant t_key();
  void t_next();
  void t_rewind();
  bool t_valid();
  void t_send(CVarRef v);
  void t_raise(CVarRef v);
  String t_getorigfuncname();
  String t_getcalledclass();

  c_Continuation* clone();

  static c_Continuation* alloc(const Func* origFunc, const Func* genFunc) {
    assert(origFunc);
    assert(genFunc);

    size_t arOffset = getArOffset(genFunc);
    size_t objectSize = arOffset + sizeof(ActRec);
    c_Continuation* cont = (c_Continuation*)ALLOCOBJSZ(objectSize);
    new ((void *)cont) c_Continuation();
    cont->m_origFunc = const_cast<Func*>(origFunc);
    cont->m_arPtr = (ActRec*)(uintptr_t(cont) + arOffset);
    memset((void*)((uintptr_t)cont + sizeof(c_Continuation)), 0,
           arOffset - sizeof(c_Continuation));
    assert(cont->getObjectSize() == objectSize);
    return cont;
  }

  static size_t getArOffset(const Func* genFunc) {
    size_t arOffset =
      sizeof(c_Continuation) +
      sizeof(Iter) * genFunc->numIterators() +
      sizeof(TypedValue) * genFunc->numLocals();
    arOffset += sizeof(TypedValue) - 1;
    arOffset &= ~(sizeof(TypedValue) - 1);
    return arOffset;
  }

public:
  void call_next();
  void call_send(Cell& v);
  void call_raise(ObjectData* e);

  inline void preNext() {
    if (done()) {
      throw_exception(Object(SystemLib::AllocExceptionObject(
                               "Continuation is already finished")));
    }
    if (running()) {
      throw_exception(Object(SystemLib::AllocExceptionObject(
                               "Continuation is already running")));
    }
    setRunning();
    start();
  }

  inline void startedCheck() {
    if (!started()) {
      throw_exception(
        Object(SystemLib::AllocExceptionObject("Need to call next() first")));
    }
  }

private:
  size_t getObjectSize() {
    return (char*)(m_arPtr + 1) - (char*)this;
  }

  void dupContVar(const StringData *name, TypedValue *src);
  void copyContinuationVars(ActRec *fp);

public:
  /* 32-bit o_id from ObjectData */
  int32_t m_label;
  int64_t m_index;
  Variant m_key;
  Variant m_value;
  Func *m_origFunc;

  ActRec* m_arPtr;
  p_ContinuationWaitHandle m_waitHandle;

  String& getCalledClass() { not_reached(); }

  ActRec* actRec() {
    return m_arPtr;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CONTINUATION_H_
