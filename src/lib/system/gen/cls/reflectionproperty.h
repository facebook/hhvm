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

#ifndef __GENERATED_cls_reflectionproperty_h__
#define __GENERATED_cls_reflectionproperty_h__

#include <cls/reflector.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 496 */
class c_reflectionproperty : virtual public c_reflector {
  BEGIN_CLASS_MAP(reflectionproperty)
    PARENT_CLASS(reflector)
  END_CLASS_MAP(reflectionproperty)
  DECLARE_CLASS(reflectionproperty, ReflectionProperty, ObjectData)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: Variant m_info;
  public: Variant m_name;
  public: Variant m_class;
  public: void t___construct(Variant v_cls, Variant v_name);
  public: ObjectData *create(Variant v_cls, Variant v_name);
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: String t___tostring();
  public: static Variant ti_export(const char* cls, Variant v_cls, Variant v_name, CVarRef v_ret);
  public: Variant t_getname();
  public: bool t_ispublic();
  public: bool t_isprivate();
  public: bool t_isprotected();
  public: Variant t_isstatic();
  public: Variant t_isdefault();
  public: void t_setaccessible();
  public: Variant t_getmodifiers();
  public: Variant t_getvalue(CVarRef v_obj);
  public: Variant t_setvalue(CVarRef v_obj, CVarRef v_value);
  public: Variant t_getdeclaringclass();
  public: Variant t_getdoccomment();
  public: static Variant t_export(CVarRef v_cls, CVarRef v_name, CVarRef v_ret) { return ti_export("reflectionproperty", v_cls, v_name, v_ret); }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_reflectionproperty_h__
