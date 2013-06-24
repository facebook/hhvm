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


#include "hphp/runtime/base/base_includes.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS_BUILTIN(Continuation);
FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);
p_Continuation f_hphp_create_continuation(CStrRef clsname, CStrRef funcname, CStrRef origFuncName, CArrRef args = null_array);

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
  bool done() const { return o_subclassData.u8[0]; }
  bool running() const { return o_subclassData.u8[1]; }
  void setDone(bool done) { o_subclassData.u8[0] = done; }
  void setRunning(bool running) { o_subclassData.u8[1] = running; }
  static constexpr uint doneOffset() {
    return offsetof(c_Continuation, o_subclassData);
  }
  static constexpr uint runningOffset() {
    return offsetof(c_Continuation, o_subclassData) + 1;
  }

  void t___construct();
  void t_update(int64_t label, CVarRef value);
  Object t_getwaithandle();
  int64_t t_getlabel();
  Variant t_current();
  int64_t t_key();
  void t_next();
  void t_rewind();
  bool t_valid();
  void t_send(CVarRef v);
  void t_raise(CVarRef v);
  String t_getorigfuncname();
  String t_getcalledclass();
  Variant t___clone();

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

protected: virtual bool php_sleep(Variant &ret);
public:
  void call_next();
  void call_send(TypedValue* v);
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
    setRunning(true);
    ++m_index;
  }

  inline void startedCheck() {
    if (m_index < 0LL) {
      throw_exception(
        Object(SystemLib::AllocExceptionObject("Need to call next() first")));
    }
  }

private:
  size_t getObjectSize() {
    return (char*)(m_arPtr + 1) - (char*)this;
  }

public:
  /* 32-bit o_id from ObjectData */
  int32_t m_label;
  int64_t m_index;
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
// class DummyContinuation

FORWARD_DECLARE_CLASS_BUILTIN(DummyContinuation);
class c_DummyContinuation : public ExtObjectData {
 public:
  DECLARE_CLASS(DummyContinuation, DummyContinuation, ObjectData)

  // need to implement
  public: c_DummyContinuation(Class* cls = c_DummyContinuation::s_cls);
  public: ~c_DummyContinuation();
  public: void t___construct();
  public: Variant t_current();
  public: int64_t t_key();
  public: void t_next();
  public: void t_rewind();
  public: bool t_valid();

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CONTINUATION_H_
