/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/base/dynamic_object_data.h>

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
  EvalObjectData(ClassEvalState &cls);
  ObjectData *dynCreate(CArrRef params, bool init /* = true */);
  void dynConstruct(CArrRef params);
  virtual void init();
  virtual void destruct();

  // properties
  virtual void o_get(std::vector<ArrayElement *> &props) const;
  virtual Variant o_get(CStrRef s, int64 hash);
  virtual Variant o_getUnchecked(CStrRef s, int64 hash);
  virtual Variant &o_lval(CStrRef s, int64 hash);
  virtual Variant o_set(CStrRef s, int64 hash, CVarRef v,
                        bool forInit  = false);

   // methods
  virtual const char *o_getClassName() const;
  virtual const MethodStatement *getMethodStatement(const char* name) const;

  virtual bool o_instanceof(const char *s) const;

  virtual Variant o_invoke(const char *s, CArrRef params, int64 hash,
                           bool fatal = true);
  virtual Variant o_invoke_ex(const char *clsname, const char *s,
                              CArrRef params, int64 hash,
                              bool fatal /* = false */);
  virtual Variant o_invoke_few_args(const char *s, int64 hash, int count,
                                    CVarRef a0 = null_variant,
                                    CVarRef a1 = null_variant,
                                    CVarRef a2 = null_variant
#if INVOKE_FEW_ARGS_COUNT > 3
                                    ,CVarRef a3 = null_variant,
                                    CVarRef a4 = null_variant,
                                    CVarRef a5 = null_variant
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                                    ,CVarRef a6 = null_variant,
                                    CVarRef a7 = null_variant,
                                    CVarRef a8 = null_variant,
                                    CVarRef a9 = null_variant
#endif
);

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
  virtual Variant t___set_state(Variant v_properties);
  virtual String t___tostring();
  virtual Variant t___clone();
  virtual Variant &___lval(Variant v_name);
  virtual Variant &___offsetget_lval(Variant v_name);

protected:
  virtual ObjectData* cloneImpl();

  virtual bool php_sleep(Variant &ret);

private:
  ClassEvalState &m_cls;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __HPHP_EVAL_OBJECT_DATA_H__ */
