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

#ifndef __GENERATED_CLS_REFLECTIONCLASS_H__
#define __GENERATED_CLS_REFLECTIONCLASS_H__

#include <cls/reflector.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 200 */
class c_reflectionclass : virtual public c_reflector {
  BEGIN_CLASS_MAP(reflectionclass)
    PARENT_CLASS(reflector)
  END_CLASS_MAP(reflectionclass)
  DECLARE_CLASS(reflectionclass, ReflectionClass, ObjectData)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: Variant m_name;
  public: Variant m_info;
  public: void t___construct(Variant v_name);
  public: ObjectData *create(Variant v_name);
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: String t___tostring();
  public: static Variant ti_export(const char* cls, CVarRef v_name, CVarRef v_ret);
  public: Variant t_getname();
  public: Variant t_isinternal();
  public: bool t_isuserdefined();
  public: bool t_isinstantiable();
  public: bool t_hasconstant(CVarRef v_name);
  public: bool t_hasmethod(CVarRef v_name);
  public: bool t_hasproperty(CVarRef v_name);
  public: Variant t_getfilename();
  public: Variant t_getstartline();
  public: Variant t_getendline();
  public: Variant t_getdoccomment();
  public: Variant t_getconstructor();
  public: p_reflectionmethod t_getmethod(CVarRef v_name);
  public: Array t_getmethods();
  public: p_reflectionproperty t_getproperty(CVarRef v_name);
  public: Array t_getproperties();
  public: Variant t_getconstants();
  public: Variant t_getconstant(CVarRef v_name);
  public: Variant t_getinterfaces();
  public: Variant t_isinterface();
  public: Variant t_isabstract();
  public: Variant t_isfinal();
  public: Variant t_getmodifiers();
  public: bool t_isinstance(CVarRef v_obj);
  public: Object t_newinstance(int num_args, Array args = Array());
  public: Object t_newinstanceargs(CVarRef v_args);
  public: Variant t_getparentclass();
  public: Variant t_issubclassof(Variant v_cls);
  public: Variant t_getstaticproperties();
  public: Variant t_getstaticpropertyvalue(CVarRef v_name, CVarRef v_default = null_variant);
  public: void t_setstaticpropertyvalue(CVarRef v_name, CVarRef v_value);
  public: Variant t_getdefaultproperties();
  public: Variant t_isiterateable();
  public: bool t_implementsinterface(Variant v_cls);
  public: Variant t_getextension();
  public: Variant t_getextensionname();
  public: static Variant t_export(CVarRef v_name, CVarRef v_ret) { return ti_export("reflectionclass", v_name, v_ret); }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_CLS_REFLECTIONCLASS_H__
