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

  static ReflectionFuncHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionFuncHandle>(obj);
  }

  static const Func* GetFuncFor(ObjectData* obj) {
    return Native::data<ReflectionFuncHandle>(obj)->getFunc();
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

/* A ReflectionClassHandle is a NativeData object wrapping a Class* for the
 * purposes of ReflectionClass. */
extern const StaticString s_ReflectionClassHandle;
class ReflectionClassHandle {
 public:
  ReflectionClassHandle(): m_cls(nullptr) {}
  explicit ReflectionClassHandle(const Class* cls): m_cls(cls) {};
  ReflectionClassHandle(const ReflectionClassHandle&) = delete;
  ReflectionClassHandle& operator=(const ReflectionClassHandle& that_) {
    m_cls = that_.m_cls;
    return *this;
  }
  ~ReflectionClassHandle() {}

  static ReflectionClassHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionClassHandle>(obj);
  }

  static const Class* GetClassFor(ObjectData* obj) {
    return Native::data<ReflectionClassHandle>(obj)->getClass();
  }

  const Class* getClass() { return m_cls; }
  void setClass(const Class* cls) {
    assert(cls != nullptr);
    assert(m_cls == nullptr);
    m_cls = cls;
  }

 private:
  const Class* m_cls{nullptr};
};

namespace DebuggerReflection {
Array get_function_info(const String& name);
Array get_class_info(const String& name);
}

// These helpers are shared by an FB-specific extension.
Class* get_cls(const Variant& class_or_object);
const Func* get_method_func(const Class* cls, const String& meth_name);
Variant default_arg_from_php_code(const Func::ParamInfo& fpi, const Func* func);
bool resolveDefaultParameterConstant(const char *value, int64_t valueLen,
                                     Variant &cns);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_REFLECTION_H_
