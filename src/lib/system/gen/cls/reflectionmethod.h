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

#ifndef __GENERATED_CLS_REFLECTIONMETHOD_H__
#define __GENERATED_CLS_REFLECTIONMETHOD_H__

#include <cls/reflectionfunctionabstract.h>
#include <cls/reflector.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 594 */
class c_reflectionmethod : virtual public c_reflectionfunctionabstract, virtual public c_reflector {
  BEGIN_CLASS_MAP(reflectionmethod)
    PARENT_CLASS(reflectionfunctionabstract)
    PARENT_CLASS(reflector)
  END_CLASS_MAP(reflectionmethod)
  DECLARE_CLASS(reflectionmethod, ReflectionMethod, reflectionfunctionabstract)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: Variant m_name;
  public: Variant m_class;
  public: void t___construct(Variant v_cls, Variant v_name);
  public: ObjectData *create(Variant v_cls, Variant v_name);
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: String t___tostring();
  public: static Variant ti_export(const char* cls, Variant v_cls, Variant v_name, CVarRef v_ret);
  public: Variant t_invoke(int num_args, CVarRef v_obj, Array args = Array());
  public: Variant t_invokeargs(CVarRef v_obj, CVarRef v_args);
  public: Variant t_isfinal();
  public: Variant t_isabstract();
  public: bool t_ispublic();
  public: bool t_isprivate();
  public: bool t_isprotected();
  public: Variant t_isstatic();
  public: bool t_isconstructor();
  public: bool t_isdestructor();
  public: Variant t_getmodifiers();
  public: Variant t_getclosure();
  public: Variant t_getdeclaringclass();
  public: static Variant t_export(CVarRef v_cls, CVarRef v_name, CVarRef v_ret) { return ti_export("reflectionmethod", v_cls, v_name, v_ret); }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_CLS_REFLECTIONMETHOD_H__
