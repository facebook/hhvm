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

#ifndef __GENERATED_CLS_REFLECTIONOBJECT_H__
#define __GENERATED_CLS_REFLECTIONOBJECT_H__

#include <cls/reflectionclass.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 479 */
class c_reflectionobject : virtual public c_reflectionclass {
  BEGIN_CLASS_MAP(reflectionobject)
    PARENT_CLASS(reflector)
    PARENT_CLASS(reflectionclass)
  END_CLASS_MAP(reflectionobject)
  DECLARE_CLASS(reflectionobject, ReflectionObject, reflectionclass)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: void t___construct(Variant v_obj);
  public: ObjectData *create(Variant v_obj);
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: static Variant ti_export(const char* cls, Variant v_obj, CVarRef v_ret);
  public: static Variant t_export(CVarRef v_obj, CVarRef v_ret) { return ti_export("reflectionobject", v_obj, v_ret); }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_CLS_REFLECTIONOBJECT_H__
