/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_CONTINUATION_H__
#define __EXT_CONTINUATION_H__


#include <runtime/base/base_includes.h>
#include <system/lib/systemlib.h>

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
    DELETEOBJSZ(sizeForLocalsAndIters(this_->m_vmFunc->numLocals(),
                                      this_->m_vmFunc->numIterators()))(this_);
  }

  // need to implement
  public: c_Continuation(VM::Class* cls = c_Continuation::s_cls);
  public: ~c_Continuation();
  public: void t___construct(int64_t func, CStrRef origFuncName, CVarRef obj = uninit_null(), CArrRef args = null_array);
  public: void t_update(int64_t label, CVarRef value);
  public: Object t_getwaithandle();
  public: int64_t t_getlabel();
  public: int64_t t_num_args();
  public: Array t_get_args();
  public: Variant t_get_arg(int64_t id);
  public: Variant t_current();
  public: int64_t t_key();
  public: void t_next();
  public: void t_rewind();
  public: bool t_valid();
  public: void t_send(CVarRef v);
  public: void t_raise(CVarRef v);
  public: void t_raised();
  public: String t_getorigfuncname();
  public: Variant t___clone();

  static c_Continuation* alloc(VM::Class* cls, int nLocals, int nIters) {
    c_Continuation* cont =
      (c_Continuation*)ALLOCOBJSZ(sizeForLocalsAndIters(nLocals, nIters));
    new ((void *)cont) c_Continuation(cls);
    cont->m_localsOffset = sizeof(c_Continuation) + sizeof(VM::Iter) * nIters;
    cont->m_arPtr = (VM::ActRec*)(cont->locals() + nLocals);

    memset((void*)((uintptr_t)cont + sizeof(c_Continuation)), 0,
           sizeof(TypedValue) * nLocals + sizeof(VM::Iter) * nIters);
    return cont;
  }

protected: virtual bool php_sleep(Variant &ret);
public:
  void call_next();
  void call_send(TypedValue* v);
  void call_raise(ObjectData* e);

  inline void preNext() {
    if (m_done) {
      throw_exception(Object(SystemLib::AllocExceptionObject(
                               "Continuation is already finished")));
    }
    if (m_running) {
      throw_exception(Object(SystemLib::AllocExceptionObject(
                               "Continuation is already running")));
    }
    m_running = true;
    ++m_index;
  }

  inline void startedCheck() {
    if (m_index < 0LL) {
      throw_exception(
        Object(SystemLib::AllocExceptionObject("Need to call next() first")));
    }
  }

public:
  Object m_obj;
  Array m_args;
  int64_t m_index;
  Variant m_value;
  Variant m_received;
  String m_origFuncName;
  bool m_done;
  bool m_running;
  bool m_should_throw;

  int m_localsOffset;
  VM::Func *m_vmFunc;
  int64_t m_label;
  VM::ActRec* m_arPtr;

  p_ContinuationWaitHandle m_waitHandle;

  SmartPtr<HphpArray> m_VMStatics;

  String& getCalledClass() { not_reached(); }

  HphpArray* getStaticLocals();
  static size_t sizeForLocalsAndIters(int nLocals, int nIters) {
    return (sizeof(c_Continuation) + sizeof(TypedValue) * nLocals +
            sizeof(VM::Iter) * nIters + sizeof(VM::ActRec));
  }
  VM::ActRec* actRec() {
    return m_arPtr;
  }
  TypedValue* locals() {
    return (TypedValue*)(uintptr_t(this) + m_localsOffset);
  }
};

///////////////////////////////////////////////////////////////////////////////
// class DummyContinuation

FORWARD_DECLARE_CLASS_BUILTIN(DummyContinuation);
class c_DummyContinuation : public ExtObjectData {
 public:
  DECLARE_CLASS(DummyContinuation, DummyContinuation, ObjectData)

  // need to implement
  public: c_DummyContinuation(VM::Class* cls = c_DummyContinuation::s_cls);
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

#endif // __EXT_CONTINUATION_H__
