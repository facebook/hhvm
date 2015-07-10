/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/native.h"
#include "hphp/util/logger.h"
#include <dlfcn.h>
#include <vector>

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

hphp_string_set BuiltinSymbols::s_superGlobals;

///////////////////////////////////////////////////////////////////////////////

int BuiltinSymbols::NumGlobalNames() {
  return sizeof(BuiltinSymbols::GlobalNames) /
    sizeof(BuiltinSymbols::GlobalNames[0]);
}

const StaticString
  s_fb_call_user_func_safe("fb_call_user_func_safe"),
  s_fb_call_user_func_safe_return("fb_call_user_func_safe_return"),
  s_fb_call_user_func_array_safe("fb_call_user_func_array_safe");

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
  }

  f->setClassInfoAttribute(attrs);
  f->setDocComment(method->docComment);

  if (!isMethod && (attrs & ClassInfo::HasOptFunction)) {
    // Legacy optimization functions
    if (method->name.same(s_fb_call_user_func_safe) ||
        method->name.same(s_fb_call_user_func_safe_return) ||
        method->name.same(s_fb_call_user_func_array_safe)) {
      f->setOptFunction(hphp_opt_fb_call_user_func);
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
  if (attrs & ClassInfo::RefVariableArguments) {
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
  if (attrs & ClassInfo::NoFCallBuiltin) {
    f->setNoFCallBuiltin();
  }
  if ((attrs & ClassInfo::AllowOverride) && !isMethod) {
    f->setAllowOverride();
  }

  FunctionScope::RecordFunctionInfo(f->getScopeName(), f);
  return f;
}

void BuiltinSymbols::ImportExtFunctions(AnalysisResultPtr ar,
                                        ClassInfo *cls) {
  const ClassInfo::MethodVec &methods = cls->getMethodsVec();
  for (auto it = methods.begin(); it != methods.end(); ++it) {
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
    auto modifiers =
      std::make_shared<ModifierExpression>(BlockScopePtr(), Location::Range());
    if (attrs & ClassInfo::IsPrivate) {
      modifiers->add(T_PRIVATE);
    } else if (attrs & ClassInfo::IsProtected) {
      modifiers->add(T_PROTECTED);
    }
    if (attrs & ClassInfo::IsStatic) {
      modifiers->add(T_STATIC);
    }

    dest->add(pinfo->name.data(), false, ar, ExpressionPtr(), modifiers);
  }
}

void BuiltinSymbols::ImportNativeConstants(AnalysisResultPtr ar,
                                           ConstantTablePtr dest) {
  for (auto cnsPair : Native::getConstants()) {
    ExpressionPtr e(Expression::MakeScalarExpression(
                      ar, ar, Location::Range(), tvAsVariant(&cnsPair.second)));

    dest->add(cnsPair.first->data(), e, ar, e);

    if ((cnsPair.second.m_type == KindOfUninit) &&
         cnsPair.second.m_data.pref) {
      // Callback based constant
      dest->setDynamic(ar, cnsPair.first->data());
    }
  }
}

void BuiltinSymbols::ImportExtConstants(AnalysisResultPtr ar,
                                        ConstantTablePtr dest,
                                        ClassInfo *cls) {
  for (auto cinfo : cls->getConstantsVec()) {
    auto e = Expression::MakeScalarExpression(ar, ar, Location::Range(),
                                              cinfo->getValue());
    dest->add(cinfo->name.data(), e, ar, e);
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

  auto cl = std::make_shared<ClassScope>(
    ar, cls->getName().toCppString(), parent.toCppString(),
    stdIfaces, methods);

  for (uint32_t i = 0; i < methods.size(); ++i) {
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
  ImportNativeConstants(ar, cns);
  ImportExtConstants(ar, cns, ClassInfo::GetSystem());
  ImportExtClasses(ar);

  Array constants = ClassInfo::GetSystemConstants();
  for (ArrayIter it = constants.begin(); it; ++it) {
    const Variant& key = it.first();
    if (!key.isString()) continue;
    std::string name = key.toCStrRef().data();
    if (cns->getSymbol(name)) continue;
    if (name == "true" || name == "false" || name == "null") continue;
    const Variant& value = it.secondRef();
    if (!value.isInitialized() || value.isObject()) continue;
    ExpressionPtr e = Expression::MakeScalarExpression(
      ar, ar, Location::Range(), value);
    cns->add(key.toCStrRef().data(), e, ar, e);
  }
  for (int i = 0, n = NumGlobalNames(); i < n; ++i) {
    ar->getVariables()->add(GlobalNames[i], false, ar,
                            ConstructPtr(), ModifierExpressionPtr());
  }

  cns->setDynamic(ar, "PHP_BINARY");
  cns->setDynamic(ar, "PHP_BINDIR");
  cns->setDynamic(ar, "PHP_OS");
  cns->setDynamic(ar, "PHP_SAPI");
  cns->setDynamic(ar, "SID");

  for (auto sym : cns->getSymbols()) {
    sym->setSystem();
  }

  // Systemlib files were all parsed by hphp_process_init

  const StringToFileScopePtrMap &files = ar->getAllFiles();
  for (const auto& file : files) {
    file.second->setSystem();

    const auto& classes = file.second->getClasses();
    for (const auto& clsVec : classes) {
      assert(clsVec.second.size() == 1);
      auto cls = clsVec.second[0];
      if (auto nativeConsts = Native::getClassConstants(
            String(cls->getScopeName()).get())) {
        for (auto cnsMap : *nativeConsts) {
          auto tv = cnsMap.second;
          auto e = Expression::MakeScalarExpression(ar, ar, Location::Range(),
                                                    tvAsVariant(&tv));
          cls->getConstants()->add(cnsMap.first->data(), e, ar, e);
        }
      }
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
    s_superGlobals.insert("_SERVER");
    s_superGlobals.insert("_GET");
    s_superGlobals.insert("_POST");
    s_superGlobals.insert("_COOKIE");
    s_superGlobals.insert("_FILES");
    s_superGlobals.insert("_ENV");
    s_superGlobals.insert("_REQUEST");
    s_superGlobals.insert("_SESSION");
  }
}

bool BuiltinSymbols::IsSuperGlobal(const std::string &name) {
  return s_superGlobals.count(name);
}
