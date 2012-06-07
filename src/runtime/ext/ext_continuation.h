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
p_Continuation f_hphp_create_continuation(CStrRef clsname, CStrRef funcname, CStrRef origFuncName, CArrRef args = null_array);
void f_hphp_pack_continuation(CObjRef continuation, int64 label, CVarRef value);
void f_hphp_unpack_continuation(CObjRef continuation);

///////////////////////////////////////////////////////////////////////////////
// class Continuation

#define LABEL_DECL public: int64 m_label;

class c_Continuation : public ExtObjectData {
 public:
  DECLARE_CLASS(Continuation, Continuation, ObjectData)

  // properties
  public: Object m_obj;
  public: Array m_args;
#ifndef HHVM
  LABEL_DECL
#endif
  public: int64 m_index;
  public: Variant m_value;
  public: Variant m_received;
  public: String m_origFuncName;
  public: String m_called_class;
  public: bool m_done;
  public: bool m_running;

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
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);

  // implemented by HPHP
  public: c_Continuation *create(int64 func, int64 extra, bool isMethod, String origFuncName, Variant obj = null, Array args = null_array);
  public: static const ClassPropTable os_prop_table;
  public: void setCalledClass(CStrRef cls) {
    const_assert(!hhvm);
    m_called_class = cls;
  }
protected: virtual bool php_sleep(Variant &ret);
private:
  template<typename FI> void nextImpl(FI& fi);

public:
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

  inline void nextCheck() {
    if (m_index < 0LL) {
      throw_exception(
        Object(SystemLib::AllocExceptionObject("Need to call next() first")));
    }
  }


  bool m_should_throw;
  bool m_isMethod;
  const CallInfo *m_callInfo;
  union {
    void *m_extra;
    VM::Func *m_vmFunc;
  };
#ifdef HHVM
  LABEL_DECL
#endif
};
#undef LABEL_DECL

///////////////////////////////////////////////////////////////////////////////
// class GenericContinuation

FORWARD_DECLARE_CLASS_BUILTIN(GenericContinuation);
class c_GenericContinuation : public c_Continuation, public Sweepable {
 public:
  DECLARE_CLASS(GenericContinuation, GenericContinuation, Continuation)

  // properties
public:
  TypedValue* m_locals;
  bool m_hasExtraVars;
  int m_nLocals;
  Array m_vars;
  intptr_t m_vmCalledClass; // Stored with 1 in its low bit
  VM::Class* getVMCalledClass() {
    return (VM::Class*)(m_vmCalledClass & ~0x1ll);
  }

  LVariableTable m_statics;
  static const ClassPropTable os_prop_table;
private:
  SmartPtr<HphpArray> m_VMStatics;

  // need to implement
  public: c_GenericContinuation(const ObjectStaticCallbacks *cb = &cw_GenericContinuation);
  public: ~c_GenericContinuation();
  public: void t___construct(int64 func, int64 extra, bool isMethod, CStrRef origFuncName, CArrRef vars, CVarRef obj = null, CArrRef args = null_array);
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: void t_update(int64 label, CVarRef value, CArrRef vars);
  DECLARE_METHOD_INVOKE_HELPERS(update);
  public: Array t_getvars();
  DECLARE_METHOD_INVOKE_HELPERS(getvars);
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);

  // implemented by HPHP
  public: c_GenericContinuation *create(int64 func, int64 extra, bool isMethod, String origFuncName, Array vars, Variant obj = null, Array args = null_array);
public:
  HphpArray* getStaticLocals();
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
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);

  // implemented by HPHP
  public: c_DummyContinuation *create();

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_CONTINUATION_H__
