/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_REFLECTION_H_
#define incl_HPHP_EXT_REFLECTION_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(hphp_get_extension_info, const String& name);
Array HHVM_FUNCTION(hphp_get_method_info, const Variant& cls, const String& name);
Array HHVM_FUNCTION(hphp_get_closure_info, const Object& closure);
Array HHVM_FUNCTION(hphp_get_class_info, const Variant& name);
Array HHVM_FUNCTION(hphp_get_function_info, const String& name);
Variant HHVM_FUNCTION(hphp_invoke, const String& name, const Variant& params);
Variant HHVM_FUNCTION(hphp_invoke_method, const Variant& obj, const String& cls,
                                          const String& name, const Variant& params);
Object HHVM_FUNCTION(hphp_create_object, const String& name, const Variant& params);
Object HHVM_FUNCTION(hphp_create_object_without_constructor,
                      const String& name);
Variant HHVM_FUNCTION(hphp_get_property, const Object& obj, const String& cls,
                                         const String& prop);
void HHVM_FUNCTION(hphp_set_property, const Object& obj, const String& cls,
                                      const String& prop, const Variant& value);
Variant HHVM_FUNCTION(hphp_get_static_property, const String& cls,
                                                const String& prop, bool force);
void HHVM_FUNCTION(hphp_set_static_property, const String& cls,
                                             const String& prop, const Variant& value,
                                             bool force);
String HHVM_FUNCTION(hphp_get_original_class_name, const String& name);
bool HHVM_FUNCTION(hphp_scalar_typehints_enabled);

class Reflection {
 public:
  static HPHP::Class* s_ReflectionExceptionClass;
  static ObjectData* AllocReflectionExceptionObject(const Variant& message);
};


/* A ReflectionFuncHandle is a NativeData object wrapping a Func*
 * for the purposes of ReflectionFunction and ReflectionMethod. */
extern const StaticString s_ReflectionFuncHandle;
class ReflectionFuncHandle {
 public:
  ReflectionFuncHandle(): m_func(nullptr) {}
  explicit ReflectionFuncHandle(const Func* func): m_func(func) {};
  ReflectionFuncHandle(const ReflectionFuncHandle&) = delete;
  ReflectionFuncHandle& operator=(const ReflectionFuncHandle& other) {
    m_func = other.m_func;
    return *this;
  }
  ~ReflectionFuncHandle() {}

  static ReflectionFuncHandle* Get(Object obj) {
    if (obj.isNull()) {
      raise_error("NULL object passed");
      return nullptr;
    }
    auto ret = Native::data<ReflectionFuncHandle>(obj.get());
    return ret;
  }

  static const Func* GetFuncFor(Object obj) {
    auto handle = ReflectionFuncHandle::Get(obj);
    assert(handle);
    return handle->getFunc();
  }

  const Func* getFunc() { return m_func; }
  void setFunc(const Func* func) {
    assert(func != nullptr);
    assert(m_func == nullptr);
    m_func = func;
  }

 private:
  const Func* m_func{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_REFLECTION_H_
