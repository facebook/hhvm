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

#ifndef __HPHP_DYNAMIC_OBJECT_DATA_H__
#define __HPHP_DYNAMIC_OBJECT_DATA_H__

#include <runtime/base/types.h>
#include <runtime/base/object_data.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class DynamicObjectData : public ObjectData {
 public:
  DynamicObjectData(const char* pname, ObjectData *r = NULL);

  virtual void init();
  virtual void dynConstruct(CArrRef params);
  virtual void dynConstructFromEval(Eval::VariableEnvironment &env,
                                    const Eval::FunctionCallExpression *call);
  virtual void getConstructor(MethodCallPackage &mcp);

  virtual void destruct();
  virtual void setRoot(ObjectData *r);
  virtual ObjectData *getRoot();

  // properties
  virtual Array o_toArray() const;
  virtual Array o_getDynamicProperties() const;
  virtual Variant *o_realProp(CStrRef prop, int flags,
                              CStrRef context = null_string) const;
  virtual Variant *o_realPropPublic(CStrRef prop, int flags) const;
  virtual void o_getArray(Array &props, bool pubOnly = false) const;
  virtual void o_setArray(CArrRef props);

  // methods
  virtual bool o_get_call_info(MethodCallPackage &info, int64 hash = -1);
  virtual bool o_get_call_info_ex(const char *clsname,
      MethodCallPackage &info, int64 hash = -1);

  virtual Variant doCall(Variant v_name, Variant v_arguments, bool fatal);
  virtual Variant doRootCall(Variant v_name, Variant v_arguments, bool fatal);

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

  void setParent(Object p) { parent = p; }
  ObjectData *getRedeclaredParent() const { return parent.get(); }

 protected:
  void cloneSet(ObjectData *clone);
  ObjectData* root;
  Object parent;
};

typedef DynamicObjectData c_DynamicObjectData; // purely for easier code generation

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_DYNAMIC_OBJECT_DATA_H__
