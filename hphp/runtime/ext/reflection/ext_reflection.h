/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Reflection {
  [[noreturn]]
  static void ThrowReflectionExceptionObject(const Variant& message);
};

struct ReflectionFileHandle : SystemLib::ClassLoader<"ReflectionFile"> {
  ReflectionFileHandle(): m_unit(nullptr) {}
  explicit ReflectionFileHandle(const Unit* unit): m_unit(unit) {};
  ReflectionFileHandle(const ReflectionFileHandle&) = delete;
  ReflectionFileHandle& operator=(const ReflectionFileHandle& other) {
    m_unit = other.m_unit;
    return *this;
  }

  static ReflectionFileHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionFileHandle>(obj);
  }

  static const Unit* GetUnitFor(ObjectData* obj) {
    return Native::data<ReflectionFileHandle>(obj)->getUnit();
  }

  const Unit* getUnit() { return m_unit; }
  void setUnit(const Unit* unit) {
    assertx(unit != nullptr);
    assertx(m_unit == nullptr);
    m_unit = unit;
  }

 private:
  LowPtr<const Unit> m_unit{nullptr};
};

/* A ReflectionModuleHandle is a NativeData object wrapping a Module*
 * for the purposes of ReflectionModule. */
struct ReflectionModuleHandle : SystemLib::ClassLoader<"ReflectionModule"> {
  ReflectionModuleHandle(): m_module(nullptr) {}
  explicit ReflectionModuleHandle(const Module* module): m_module(module) {}
  ReflectionModuleHandle(const ReflectionModuleHandle&) = delete;
  ReflectionModuleHandle& operator=(const ReflectionModuleHandle& other) {
    m_module = other.m_module;
    return *this;
  }

  static ReflectionModuleHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionModuleHandle>(obj);
  }

  static const Module* GetModuleFor(ObjectData* obj) {
    return Native::data<ReflectionModuleHandle>(obj)->getModule();
  }

  const Module* getModule() { return m_module; }
  void setModule(const Module* module) {
    assertx(module != nullptr);
    assertx(m_module == nullptr);
    m_module = module;
  }

 private:
  const Module* m_module{nullptr};
};

/* A ReflectionFuncHandle is a NativeData object wrapping a Func*
 * for the purposes of ReflectionFunction and ReflectionMethod. */
struct ReflectionFuncHandle :
    SystemLib::ClassLoader<"ReflectionFunctionAbstract"> {
  ReflectionFuncHandle(): m_func(nullptr) {}
  explicit ReflectionFuncHandle(const Func* func): m_func(func) {};
  ReflectionFuncHandle(const ReflectionFuncHandle&) = delete;
  ReflectionFuncHandle& operator=(const ReflectionFuncHandle& other) {
    m_func = other.m_func;
    return *this;
  }

  static ReflectionFuncHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionFuncHandle>(obj);
  }

  static const Func* GetFuncFor(ObjectData* obj) {
    return Native::data<ReflectionFuncHandle>(obj)->getFunc();
  }

  const Func* getFunc() { return m_func; }
  void setFunc(const Func* func) {
    assertx(func != nullptr);
    assertx(m_func == nullptr);
    m_func = func;
  }

 private:
  LowPtr<const Func> m_func{nullptr};
};

/* A ReflectionClassHandle is a NativeData object wrapping a Class* for the
 * purposes of ReflectionClass. */
struct ReflectionClassHandle : SystemLib::ClassLoader<"ReflectionClass"> {
  ReflectionClassHandle(): m_cls(nullptr) {}
  explicit ReflectionClassHandle(const Class* cls): m_cls(cls) {};
  ReflectionClassHandle(const ReflectionClassHandle&) = delete;
  ReflectionClassHandle& operator=(const ReflectionClassHandle& that_) {
    m_cls = that_.m_cls;
    return *this;
  }
  String init(const String& name) {
    auto const cls = Class::load(name.get());
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
    assertx(cls != nullptr);
    assertx(m_cls == nullptr);
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
struct ReflectionConstHandle :
    SystemLib::ClassLoader<"ReflectionTypeConstant"> {
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
    assertx(cns != nullptr);
    assertx(m_const == nullptr);
    m_const = cns;
  }

  void setClass(const Class* cls) {
    assertx(cls != nullptr);
    assertx(m_cls == nullptr);
    m_cls = cls;
  }

 private:
  const Class::Const* m_const;
  LowPtr<const Class> m_cls;
};

/**
 * A ReflectionPropHandle is a NativeData object that represents an instance,
 * static, or dynamic property for the purposes of ReflectionProperty.
 */
struct ReflectionPropHandle : SystemLib::ClassLoader<"ReflectionProperty"> {
  enum Type : uint8_t {
    Invalid  = 0,
    Instance = 1,
    Static   = 2,
    Dynamic  = 3,
  };

  ReflectionPropHandle(): m_prop(nullptr), m_type(Invalid) {}

  Type getType() const { return m_type; }
  const Class::Prop* getProp() const {
    assertx(m_type == Instance);
    return m_prop;
  }
  const Class::SProp* getSProp() const {
    assertx(m_type == Static);
    return m_sprop;
  }

  void setInstanceProp(const Class::Prop* prop) {
    assertx(prop != nullptr);
    m_type = Instance;
    m_prop = prop;
  }
  void setStaticProp(const Class::SProp* sprop) {
    assertx(sprop != nullptr);
    m_type = Static;
    m_sprop = sprop;
  }
  void setDynamicProp() {
    m_type = Dynamic;
  }

 private:
  union {
    const Class::Prop* m_prop;
    const Class::SProp* m_sprop;
  };
  Type m_type;
};

/* A ReflectionTypeAliasHandle is a NativeData object wrapping a TypeAlias*
 * for the purposes of static ReflectionTypeAlias. */
struct ReflectionTypeAliasHandle :
    SystemLib::ClassLoader<"ReflectionTypeAlias"> {
  ReflectionTypeAliasHandle(): m_req(nullptr) {}
  explicit ReflectionTypeAliasHandle(const TypeAlias* req): m_req(req) {};

  static ReflectionTypeAliasHandle* Get(ObjectData* obj) {
    return Native::data<ReflectionTypeAliasHandle>(obj);
  }

  static const TypeAlias* GetTypeAliasFor(ObjectData* obj) {
    return Native::data<ReflectionTypeAliasHandle>(obj)->getTypeAlias();
  }

  const TypeAlias* getTypeAlias() const { return m_req; }

  void setTypeAlias(const TypeAlias* req) {
    assertx(req != nullptr);
    assertx(m_req == nullptr);
    m_req = req;
  }

 private:
  template <typename F> friend void scan(const ReflectionTypeAliasHandle&, F&);
  const TypeAlias* m_req;
};

namespace DebuggerReflection {
Array get_function_info(const String& name);
Array get_class_info(const String& name);
}

// These helpers are shared by an FB-specific extension.
Class* get_cls(const Variant& class_or_object);
const Func* get_method_func(const Class* cls, const String& meth_name);
Variant default_arg_from_php_code(const Func::ParamInfo& fpi, const Func* func,
                                  unsigned argIdx);

///////////////////////////////////////////////////////////////////////////////
}

