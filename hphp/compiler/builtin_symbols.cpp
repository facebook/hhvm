/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/type.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"
#include <dlfcn.h>

using namespace HPHP;

#define BF_COLUMN_COUNT  3
#define BF_COLUMN_NAME   0
#define BF_COLUMN_RETURN 1
#define BF_COLUMN_PARAMS 2

#define CLASS_TYPE 999

///////////////////////////////////////////////////////////////////////////////

bool BuiltinSymbols::Loaded = false;
StringBag BuiltinSymbols::s_strings;
AnalysisResultPtr BuiltinSymbols::s_systemAr;

const char *const BuiltinSymbols::GlobalNames[] = {
  "HTTP_RAW_POST_DATA",
  "_COOKIE",
  "_ENV",
  "_FILES",
  "_GET",
  "_POST",
  "_REQUEST",
  "_SERVER",
  "_SESSION",
  "argc",
  "argv",
  "http_response_header",
};

const char *BuiltinSymbols::SystemClasses[] = {
  "stdclass",
  "exception",
  "arrayaccess",
  "iterator",
  "collections",
  "reflection",
  "splobjectstorage",
  "directory",
  "splfile",
  "debugger",
  "xhprof",
  "directoryiterator",
  "soapfault",
  "fbmysqllexer",
  nullptr
};

StringToTypePtrMap BuiltinSymbols::s_superGlobals;

///////////////////////////////////////////////////////////////////////////////

int BuiltinSymbols::NumGlobalNames() {
  return sizeof(BuiltinSymbols::GlobalNames) /
    sizeof(BuiltinSymbols::GlobalNames[0]);
}

static TypePtr typePtrFromDataType(DataType dt, TypePtr unknown) {
  switch (dt) {
    case KindOfNull:    return Type::Null;
    case KindOfBoolean: return Type::Boolean;
    case KindOfInt64:   return Type::Int64;
    case KindOfDouble:  return Type::Double;
    case KindOfString:  return Type::String;
    case KindOfArray:   return Type::Array;
    case KindOfObject:  return Type::Object;
    case KindOfResource: return Type::Resource;
    case KindOfUnknown:
    default:
      return unknown;
  }
}

const StaticString
  s_fb_call_user_func_safe("fb_call_user_func_safe"),
  s_fb_call_user_func_safe_return("fb_call_user_func_safe_return"),
  s_fb_call_user_func_array_safe("fb_call_user_func_array_safe"),
  s_is_callable("is_callable"),
  s_call_user_func_array("call_user_func_array");

FunctionScopePtr BuiltinSymbols::ImportFunctionScopePtr(AnalysisResultPtr ar,
                 ClassInfo *cls, ClassInfo::MethodInfo *method) {
  int attrs = method->attribute;
  bool isMethod = cls != ClassInfo::GetSystem();
  FunctionScopePtr f(new FunctionScope(isMethod,
                                       method->name.data(),
                                       attrs & ClassInfo::IsReference));

  int reqCount = 0, totalCount = 0;
  for(auto it = method->parameters.begin();
      it != method->parameters.end(); ++it) {
    const ClassInfo::ParameterInfo *pinfo = *it;
    if (!pinfo->value || !pinfo->value[0]) {
      ++reqCount;
    }
    ++totalCount;
  }
  f->setParamCounts(ar, reqCount, totalCount);

  int idx = 0;
  for(auto it = method->parameters.begin();
      it != method->parameters.end(); ++it, ++idx) {
    const ClassInfo::ParameterInfo *pinfo = *it;
    f->setParamName(idx, pinfo->name);
    if (pinfo->attribute & ClassInfo::IsReference) {
      f->setRefParam(idx);
    }
    f->setParamType(ar, idx, typePtrFromDataType(pinfo->argType, Type::Any));
    if (pinfo->valueLen) {
      f->setParamDefault(idx, pinfo->value, pinfo->valueLen,
                         std::string(pinfo->valueText, pinfo->valueTextLen));
    }
  }

  if (method->returnType != KindOfNull) {
    f->setReturnType(ar, typePtrFromDataType(method->returnType,
                                             Type::Variant));
  }

  f->setClassInfoAttribute(attrs);
  f->setDocComment(method->docComment);

  if (!isMethod && (attrs & ClassInfo::HasOptFunction)) {
    // Legacy optimization functions
    if (method->name.same(s_fb_call_user_func_safe) ||
        method->name.same(s_fb_call_user_func_safe_return) ||
        method->name.same(s_fb_call_user_func_array_safe)) {
      f->setOptFunction(hphp_opt_fb_call_user_func);
    } else if (method->name.same(s_is_callable)) {
      f->setOptFunction(hphp_opt_is_callable);
    } else if (method->name.same(s_call_user_func_array)) {
      f->setOptFunction(hphp_opt_call_user_func);
    }
  }

  if (isMethod) {
    if (attrs & ClassInfo::IsProtected) {
      f->addModifier(T_PROTECTED);
    } else if (attrs & ClassInfo::IsPrivate) {
      f->addModifier(T_PRIVATE);
    }
    if (attrs & ClassInfo::IsStatic) {
      f->addModifier(T_STATIC);
    }
  }

  // This block of code is not needed, if BlockScope directly takes flags.
  if (attrs & ClassInfo::MixedVariableArguments) {
    f->setVariableArgument(-1);
  } else if (attrs & ClassInfo::RefVariableArguments) {
    f->setVariableArgument(1);
  } else if (attrs & ClassInfo::VariableArguments) {
    f->setVariableArgument(0);
  }
  if (attrs & ClassInfo::NoEffect) {
    f->setNoEffect();
  }
  if (attrs & ClassInfo::FunctionIsFoldable) {
    f->setIsFoldable();
  }
  if (attrs & ClassInfo::ContextSensitive) {
    f->setContextSensitive(true);
  }
  if (attrs & ClassInfo::NeedsActRec) {
    f->setNeedsActRec();
  }
  if ((attrs & ClassInfo::AllowOverride) && !isMethod) {
    f->setAllowOverride();
  }

  FunctionScope::RecordFunctionInfo(f->getName(), f);
  return f;
}

void BuiltinSymbols::ImportExtFunctions(AnalysisResultPtr ar,
                                        ClassInfo *cls) {
  const ClassInfo::MethodVec &methods = cls->getMethodsVec();
  for (auto it = methods.begin(); it != methods.end(); ++it) {
    if (((*it)->attribute & ClassInfo::ZendCompat) &&
        !Option::EnableZendCompat) {
      continue;
    }

    FunctionScopePtr f = ImportFunctionScopePtr(ar, cls, *it);
    ar->addSystemFunction(f);
  }
}

void BuiltinSymbols::ImportExtMethods(AnalysisResultPtr ar,
                                      FunctionScopePtrVec &vec,
                                      ClassInfo *cls) {
  const ClassInfo::MethodVec &methods = cls->getMethodsVec();
  for (auto it = methods.begin(); it != methods.end(); ++it) {
    FunctionScopePtr f = ImportFunctionScopePtr(ar, cls, *it);
    vec.push_back(f);
  }
}

void BuiltinSymbols::ImportExtProperties(AnalysisResultPtr ar,
                                         VariableTablePtr dest,
                                         ClassInfo *cls) {
  ClassInfo::PropertyVec src = cls->getPropertiesVec();
  for (auto it = src.begin(); it != src.end(); ++it) {
    ClassInfo::PropertyInfo *pinfo = *it;
    int attrs = pinfo->attribute;
    ModifierExpressionPtr modifiers(
      new ModifierExpression(BlockScopePtr(), LocationPtr()));
    if (attrs & ClassInfo::IsPrivate) {
      modifiers->add(T_PRIVATE);
    } else if (attrs & ClassInfo::IsProtected) {
      modifiers->add(T_PROTECTED);
    }
    if (attrs & ClassInfo::IsStatic) {
      modifiers->add(T_STATIC);
    }

    dest->add(pinfo->name.data(),
              typePtrFromDataType(pinfo->type, Type::Variant),
              false, ar, ExpressionPtr(), modifiers);
  }
}

void BuiltinSymbols::ImportExtConstants(AnalysisResultPtr ar,
                                        ConstantTablePtr dest,
                                        ClassInfo *cls) {
  ClassInfo::ConstantVec src = cls->getConstantsVec();
  for (auto it = src.begin(); it != src.end(); ++it) {
    // We make an assumption that if the constant is a callback type
    // (e.g. STDIN, STDOUT, STDERR) then it will return an Object.
    // And that if it's deferred (SID, PHP_SAPI, etc.) it'll be a String.
    ClassInfo::ConstantInfo *cinfo = *it;
    dest->add(cinfo->name.data(),
              cinfo->isDeferred() ?
              (cinfo->isCallback() ? Type::Object : Type::String) :
              typePtrFromDataType(cinfo->getValue().getType(), Type::Variant),
              ExpressionPtr(), ar, ConstructPtr());
  }
}

ClassScopePtr BuiltinSymbols::ImportClassScopePtr(AnalysisResultPtr ar,
                                                  ClassInfo *cls) {
  FunctionScopePtrVec methods;
  ImportExtMethods(ar, methods, cls);

  ClassInfo::InterfaceVec ifaces = cls->getInterfacesVec();
  String parent = cls->getParentClass();
  std::vector<std::string> stdIfaces;
  if (!parent.empty() && (ifaces.empty() || ifaces[0] != parent)) {
    stdIfaces.push_back(parent.data());
  }
  for (auto it = ifaces.begin(); it != ifaces.end(); ++it) {
    stdIfaces.push_back(it->data());
  }

  ClassScopePtr cl(new ClassScope(ar, cls->getName().data(), parent.data(),
                                  stdIfaces, methods));
  for (uint i = 0; i < methods.size(); ++i) {
    methods[i]->setOuterScope(cl);
  }

  ImportExtProperties(ar, cl->getVariables(), cls);
  ImportExtConstants(ar, cl->getConstants(), cls);
  int attrs = cls->getAttribute();
  cl->setClassInfoAttribute(attrs);
  cl->setDocComment(cls->getDocComment());
  cl->setSystem();
  return cl;
}

void BuiltinSymbols::ImportExtClasses(AnalysisResultPtr ar) {
  const ClassInfo::ClassMap &classes = ClassInfo::GetClassesMap();
  for (auto it = classes.begin(); it != classes.end(); ++it) {

    const ClassInfo *info = it->second;
    if ((info->getAttribute() & ClassInfo::ZendCompat) &&
        !Option::EnableZendCompat) {
      continue;
    }

    ClassScopePtr cl = ImportClassScopePtr(ar, it->second);
    ar->addSystemClass(cl);
  }
}

bool BuiltinSymbols::Load(AnalysisResultPtr ar) {
  if (Loaded) return true;
  Loaded = true;

  if (g_context.isNull()) init_thread_locals();
  ClassInfo::Load();

  // load extension functions first, so system/php may call them
  ImportExtFunctions(ar, ClassInfo::GetSystem());

  ConstantTablePtr cns = ar->getConstants();
  // load extension constants, classes and dynamics
  ImportExtConstants(ar, cns, ClassInfo::GetSystem());
  ImportExtClasses(ar);

  Array constants = ClassInfo::GetSystemConstants();
  LocationPtr loc(new Location);
  for (ArrayIter it = constants.begin(); it; ++it) {
    CVarRef key = it.first();
    if (!key.isString()) continue;
    std::string name = key.toCStrRef().data();
    if (cns->getSymbol(name)) continue;
    if (name == "true" || name == "false" || name == "null") continue;
    CVarRef value = it.secondRef();
    if (!value.isInitialized() || value.isObject()) continue;
    ExpressionPtr e = Expression::MakeScalarExpression(ar, ar, loc, value);
    TypePtr t =
      value.isNull()    ? Type::Null    :
      value.isBoolean() ? Type::Boolean :
      value.isInteger() ? Type::Int64   :
      value.isDouble()  ? Type::Double  :
      value.isArray()   ? Type::Array   : Type::Variant;

    cns->add(key.toCStrRef().data(), t, e, ar, e);
  }
  for (int i = 0, n = NumGlobalNames(); i < n; ++i) {
    ar->getVariables()->add(GlobalNames[i], Type::Variant, false, ar,
                            ConstructPtr(), ModifierExpressionPtr());
  }

  cns->setDynamic(ar, "PHP_BINARY", true);
  cns->setDynamic(ar, "PHP_BINDIR", true);
  cns->setDynamic(ar, "PHP_OS", true);
  cns->setDynamic(ar, "PHP_SAPI", true);
  cns->setDynamic(ar, "SID", true);

  // Systemlib files were all parsed by hphp_process_init

  const StringToFileScopePtrMap &files = ar->getAllFiles();
  for (const auto& file : files) {
    file.second->setSystem();

    const auto& classes = file.second->getClasses();
    for (const auto& clsVec : classes) {
      assert(clsVec.second.size() == 1);
      auto cls = clsVec.second[0];
      cls->setSystem();
      ar->addSystemClass(cls);
      for (const auto& func : cls->getFunctions()) {
        FunctionScope::RecordFunctionInfo(func.first, func.second);
      }
    }

    const auto& functions = file.second->getFunctions();
    for (const auto& func : functions) {
      func.second->setSystem();
      ar->addSystemFunction(func.second);
      FunctionScope::RecordFunctionInfo(func.first, func.second);
    }
  }

  return true;
}

void BuiltinSymbols::LoadSuperGlobals() {
  if (s_superGlobals.empty()) {
    s_superGlobals["_SERVER"] = Type::Variant;
    s_superGlobals["_GET"] = Type::Variant;
    s_superGlobals["_POST"] = Type::Variant;
    s_superGlobals["_COOKIE"] = Type::Variant;
    s_superGlobals["_FILES"] = Type::Variant;
    s_superGlobals["_ENV"] = Type::Variant;
    s_superGlobals["_REQUEST"] = Type::Variant;
    s_superGlobals["_SESSION"] = Type::Variant;
    s_superGlobals["http_response_header"] = Type::Variant;
  }
}

bool BuiltinSymbols::IsSuperGlobal(const std::string &name) {
  return s_superGlobals.find(name) != s_superGlobals.end();
}

TypePtr BuiltinSymbols::GetSuperGlobalType(const std::string &name) {
  StringToTypePtrMap::const_iterator iter = s_superGlobals.find(name);
  if (iter != s_superGlobals.end()) {
    return iter->second;
  }
  return TypePtr();
}
