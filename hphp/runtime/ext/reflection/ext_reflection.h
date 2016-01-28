/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/extension.h"
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

struct Reflection {
  static HPHP::Class* s_ReflectionExceptionClass;
  ATTRIBUTE_NORETURN
  static void ThrowReflectionExceptionObject(const Variant& message);
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
  LowPtr<const Func> m_func{nullptr};
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
  String init(const String& name) {
    auto const cls = Unit::loadClass(name.get());
    if (!cls) return empty_string();
    setClass(cls);
    return cls->nameStr();
  }

  static ReflectionClassHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionClassHandle>(obj);
  }

  static const Class* GetClassFor(ObjectData* obj) {
    return Native::data<ReflectionClassHandle>(obj)->getClass();
  }

  const Class* getClass() const { return m_cls; }
  void setClass(const Class* cls) {
    assert(cls != nullptr);
    assert(m_cls == nullptr);
    m_cls = cls;
  }

  Variant sleep() const {
    return String(getClass()->nameStr());
  }

  void wakeup(const Variant& content, ObjectData* obj);

 private:
  LowPtr<const Class> m_cls{nullptr};
};

/* A ReflectionConstHandle is a NativeData object wrapping a Const*
 * for the purposes of ReflectionTypeConstant. */
extern const StaticString s_ReflectionConstHandle;
struct ReflectionConstHandle {
  ReflectionConstHandle(): m_const(nullptr), m_cls(nullptr) {}
  explicit ReflectionConstHandle(const Class::Const* cns, const Class* cls):
      m_const(cns), m_cls(cls) {};

  static ReflectionConstHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionConstHandle>(obj);
  }

  static const Class::Const* GetConstFor(ObjectData* obj) {
    return Native::data<ReflectionConstHandle>(obj)->getConst();
  }

  static const Class* GetClassFor(ObjectData* obj) {
    return Native::data<ReflectionConstHandle>(obj)->getClass();
  }

  const Class::Const* getConst() const { return m_const; }
  const Class* getClass() const { return m_cls; }

  void setConst(const Class::Const* cns) {
    assert(cns != nullptr);
    assert(m_const == nullptr);
    m_const = cns;
  }

  void setClass(const Class* cls) {
    assert(cls != nullptr);
    assert(m_cls == nullptr);
    m_cls = cls;
  }

 private:
  const Class::Const* m_const;
  LowPtr<const Class> m_cls;
};

/* A ReflectionPropHandle is a NativeData object wrapping a Prop*
 * for the purposes of ReflectionProperty. */
extern const StaticString s_ReflectionPropHandle;
class ReflectionPropHandle {
 public:
  ReflectionPropHandle(): m_prop(nullptr) {}
  explicit ReflectionPropHandle(const Class::Prop* prop): m_prop(prop) {};
  ReflectionPropHandle(const ReflectionPropHandle& other) {
    m_prop = other.m_prop;
  }
  ReflectionPropHandle& operator=(const ReflectionPropHandle& other) {
    m_prop = other.m_prop;
    return *this;
  }
  ~ReflectionPropHandle() {}

  static ReflectionPropHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionPropHandle>(obj);
  }

  static const Class::Prop* GetPropFor(ObjectData* obj) {
    return Native::data<ReflectionPropHandle>(obj)->getProp();
  }

  const Class::Prop* getProp() { return m_prop; }

  void setProp(const Class::Prop* prop) {
    assert(prop != nullptr);
    assert(m_prop == nullptr);
    m_prop = prop;
  }

 private:
  const Class::Prop* m_prop{nullptr};
};

/* A ReflectionSPropHandle is a NativeData object wrapping a SProp*
 * for the purposes of static ReflectionProperty. */
extern const StaticString s_ReflectionSPropHandle;
class ReflectionSPropHandle {
 public:
  ReflectionSPropHandle(): m_sprop(nullptr) {}
  explicit ReflectionSPropHandle(const Class::SProp* sprop): m_sprop(sprop) {};
  ReflectionSPropHandle(const ReflectionSPropHandle& other) {
    m_sprop = other.m_sprop;
  }
  ReflectionSPropHandle& operator=(const ReflectionSPropHandle& other) {
    m_sprop = other.m_sprop;
    return *this;
  }
  ~ReflectionSPropHandle() {}

  static ReflectionSPropHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionSPropHandle>(obj);
  }

  static const Class::SProp* GetSPropFor(ObjectData* obj) {
    return Native::data<ReflectionSPropHandle>(obj)->getSProp();
  }

  const Class::SProp* getSProp() { return m_sprop; }

  void setSProp(const Class::SProp* sprop) {
    assert(sprop != nullptr);
    assert(m_sprop == nullptr);
    m_sprop = sprop;
  }

 private:
  const Class::SProp* m_sprop{nullptr};
};

/* A ReflectionTypeAliasHandle is a NativeData object wrapping a TypeAliasReq*
 * for the purposes of static ReflectionTypeAlias. */
struct ReflectionTypeAliasHandle {
  ReflectionTypeAliasHandle(): m_req(nullptr) {}
  explicit ReflectionTypeAliasHandle(const TypeAliasReq* req): m_req(req) {};

  static ReflectionTypeAliasHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionTypeAliasHandle>(obj);
  }

  static const TypeAliasReq* GetTypeAliasReqFor(ObjectData* obj) {
    return Native::data<ReflectionTypeAliasHandle>(obj)->getTypeAliasReq();
  }

  const TypeAliasReq* getTypeAliasReq() const { return m_req; }

  void setTypeAliasReq(const TypeAliasReq* req) {
    assert(req != nullptr);
    assert(m_req == nullptr);
    m_req = req;
  }

 private:
  template <typename F> friend void scan(const ReflectionTypeAliasHandle&, F&);
  const TypeAliasReq* m_req;
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
