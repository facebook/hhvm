/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_EVAL_OBJECT_DATA_H__
#define __HPHP_EVAL_OBJECT_DATA_H__

#include <runtime/base/dynamic_object_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class ClassStatement;
class ClassEvalState;

class EvalObjectData : public DynamicObjectData {
  DECLARE_OBJECT_ALLOCATION(EvalObjectData);
public:
  EvalObjectData(ClassEvalState &cls, const char* pname,
                 ObjectData *r = NULL);
  virtual void getConstructor(MethodCallPackage &mcp);
  virtual void init();
  virtual void destruct();

  // properties
  virtual Array o_toArray() const;
  virtual void o_getArray(Array &props, bool pubOnly = false) const;
  virtual void o_setArray(CArrRef props);
  virtual Variant *o_realPropHook(CStrRef prop, int flags,
                                  CStrRef context = null_string) const;
  void o_setPrivate(CStrRef cls, CStrRef s, CVarRef v);

  virtual Variant o_getError(CStrRef prop, CStrRef context);
  virtual Variant o_setError(CStrRef prop, CStrRef context);

   // methods
  virtual CStrRef o_getClassNameHook() const;
  virtual const MethodStatement *getMethodStatement(const char* name) const;
  virtual const MethodStatement *getConstructorStatement() const;

  virtual bool o_get_call_info_hook(const char *clsname,
                                    MethodCallPackage &mcp, int64 hash = -1);

  virtual bool o_instanceof_hook(CStrRef s) const;

  virtual Variant doCall(Variant v_name, Variant v_arguments, bool fatal);

  // magic methods
  // __construct is handled in a special way
  virtual Variant t___destruct();
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual String t___tostring();
  virtual Variant t___clone();
  virtual Variant &___offsetget_lval(Variant v_name);
  virtual const CallInfo *t___invokeCallInfoHelper(void *&extra);

  /**
   * Marshaling/Unmarshaling between request thread and fiber thread.
   */
  virtual Object fiberMarshal(FiberReferenceMap &refMap) const;
  virtual Object fiberUnmarshal(FiberReferenceMap &refMap) const;

protected:
  virtual ObjectData* clone();

  virtual bool hasCall();
  virtual bool hasCallStatic();
  virtual bool php_sleep(Variant &ret);

private:
  EvalObjectData(EvalObjectData *original); // for clone
  ClassEvalState &m_cls;
  Array m_privates;
  String m_class_name;
  MethodCallPackage m_invokeMcp;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __HPHP_EVAL_OBJECT_DATA_H__ */
