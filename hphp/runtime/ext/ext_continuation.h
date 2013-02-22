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
void f_hphp_pack_continuation(CObjRef continuation, int64 label, CVarRef value);
void f_hphp_unpack_continuation(CObjRef continuation);

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
  public: c_Continuation(const ObjectStaticCallbacks *cb = &cw_Continuation);
  public: ~c_Continuation();
  public: void t___construct(int64 func, int64 extra, bool isMethod, CStrRef origFuncName, CVarRef obj = null, CArrRef args = null_array);
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: void t_update(int64 label, CVarRef value);
  DECLARE_METHOD_INVOKE_HELPERS(update);
  public: void t_done();
  DECLARE_METHOD_INVOKE_HELPERS(done);
  public: int64 t_getlabel();
  DECLARE_METHOD_INVOKE_HELPERS(getlabel);
  public: int64 t_num_args();
  DECLARE_METHOD_INVOKE_HELPERS(num_args);
  public: Array t_get_args();
  DECLARE_METHOD_INVOKE_HELPERS(get_args);
  public: Variant t_get_arg(int64 id);
  DECLARE_METHOD_INVOKE_HELPERS(get_arg);
  public: Variant t_current();
  DECLARE_METHOD_INVOKE_HELPERS(current);
  public: int64 t_key();
  DECLARE_METHOD_INVOKE_HELPERS(key);
  public: void t_next();
  DECLARE_METHOD_INVOKE_HELPERS(next);
  public: void t_rewind();
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  public: bool t_valid();
  DECLARE_METHOD_INVOKE_HELPERS(valid);
  public: void t_send(CVarRef v);
  DECLARE_METHOD_INVOKE_HELPERS(send);
  public: void t_raise(CVarRef v);
  DECLARE_METHOD_INVOKE_HELPERS(raise);
  public: void t_raised();
  DECLARE_METHOD_INVOKE_HELPERS(raised);
  public: Variant t_receive();
  DECLARE_METHOD_INVOKE_HELPERS(receive);
  public: String t_getorigfuncname();
  DECLARE_METHOD_INVOKE_HELPERS(getorigfuncname);
  public: Variant t___clone();
  DECLARE_METHOD_INVOKE_HELPERS(__clone);

  // implemented by HPHP
  public: c_Continuation *create(int64 func, int64 extra, bool isMethod, String origFuncName, Variant obj = null, Array args = null_array);
  public: static const ClassPropTable os_prop_table;

  static c_Continuation* alloc(VM::Class* cls, int nLocals, int nIters) {
    const_assert(hhvm);
    c_Continuation* cont =
      (c_Continuation*)ALLOCOBJSZ(sizeForLocalsAndIters(nLocals, nIters));
    new ((void *)cont) c_Continuation(
      ObjectStaticCallbacks::encodeVMClass(cls));
    cont->m_localsOffset = sizeof(c_Continuation) + sizeof(VM::Iter) * nIters;
    cont->m_arPtr = (VM::ActRec*)(cont->locals() + nLocals);

    memset((void*)((uintptr_t)cont + sizeof(c_Continuation)), 0,
           sizeof(TypedValue) * nLocals + sizeof(VM::Iter) * nIters);
    return cont;
  }

protected: virtual bool php_sleep(Variant &ret);
private:
  template<typename FI> void nextImpl(FI& fi);

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
  int64 m_index;
  Variant m_value;
  Variant m_received;
  String m_origFuncName;
  bool m_done;
  bool m_running;
  bool m_should_throw;
  bool m_isMethod;

  int m_localsOffset;
  union {
    const CallInfo *m_callInfo;
    VM::Func *m_vmFunc;
  };
  int64 m_label;
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
  public: c_DummyContinuation(const ObjectStaticCallbacks *cb = &cw_DummyContinuation);
  public: ~c_DummyContinuation();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: Variant t_current();
  DECLARE_METHOD_INVOKE_HELPERS(current);
  public: int64 t_key();
  DECLARE_METHOD_INVOKE_HELPERS(key);
  public: void t_next();
  DECLARE_METHOD_INVOKE_HELPERS(next);
  public: void t_rewind();
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  public: bool t_valid();
  DECLARE_METHOD_INVOKE_HELPERS(valid);

  // implemented by HPHP
  public: c_DummyContinuation *create();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_CONTINUATION_H__
