/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <compiler/builtin_symbols.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/type.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/option.h>
#include <compiler/parser/parser.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/constant_table.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/base/class_info.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/thread_init_fini.h>
#include <util/logger.h>
#include <util/util.h>
#include <dlfcn.h>

using namespace HPHP;

#define BF_COLUMN_COUNT  3
#define BF_COLUMN_NAME   0
#define BF_COLUMN_RETURN 1
#define BF_COLUMN_PARAMS 2

#define CLASS_TYPE 999

///////////////////////////////////////////////////////////////////////////////

bool BuiltinSymbols::Loaded = false;
bool BuiltinSymbols::NoSuperGlobals = false;
StringBag BuiltinSymbols::s_strings;

StringToFunctionScopePtrMap BuiltinSymbols::s_functions;

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

StringToClassScopePtrMap BuiltinSymbols::s_classes;
VariableTablePtr BuiltinSymbols::s_variables;
ConstantTablePtr BuiltinSymbols::s_constants;
StringToTypePtrMap BuiltinSymbols::s_superGlobals;
void *BuiltinSymbols::s_handle_main = nullptr;

///////////////////////////////////////////////////////////////////////////////

int BuiltinSymbols::NumGlobalNames() {
  return sizeof(BuiltinSymbols::GlobalNames) /
    sizeof(BuiltinSymbols::GlobalNames[0]);
}

static TypePtr typePtrFromDataType(DataType dt) {
  switch (dt) {
    case KindOfNull:    return Type::Null;
    case KindOfBoolean: return Type::Boolean;
    case KindOfInt64:   return Type::Int64;
    case KindOfDouble:  return Type::Double;
    case KindOfString:  return Type::String;
    case KindOfArray:   return Type::Array;
    case KindOfObject:  return Type::Object;
    case KindOfUnknown:
    default:
      return Type::Any;
  }
}

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
    f->setParamType(ar, idx, typePtrFromDataType(pinfo->argType));
    if (pinfo->valueLen) {
      f->setParamDefault(idx, pinfo->value, pinfo->valueLen,
                         std::string(pinfo->valueText, pinfo->valueTextLen));
    }
  }

  if (method->returnType != KindOfNull) {
    f->setReturnType(ar, typePtrFromDataType(method->returnType));
  }

  f->setClassInfoAttribute(attrs);
  if (attrs & ClassInfo::HasDocComment) {
    f->setDocComment(method->docComment);
  }

  if (!isMethod && (attrs & ClassInfo::HasOptFunction)) {
    // Legacy optimization functions
    if (method->name.same("fb_call_user_func_safe") ||
        method->name.same("fb_call_user_func_safe_return") ||
        method->name.same("fb_call_user_func_array_safe")) {
      f->setOptFunction(hphp_opt_fb_call_user_func);
    } else if (method->name.same("is_callable")) {
      f->setOptFunction(hphp_opt_is_callable);
    } else if (method->name.same("call_user_func_array")) {
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
  if ((attrs & ClassInfo::IgnoreRedefinition) && !isMethod) {
    f->setIgnoreRedefinition();
  }

  FunctionScope::RecordFunctionInfo(f->getName(), f);
  return f;
}

void BuiltinSymbols::ImportExtFunctions(AnalysisResultPtr ar,
                                        StringToFunctionScopePtrMap &map,
                                        ClassInfo *cls) {
  const ClassInfo::MethodVec &methods = cls->getMethodsVec();
  for (auto it = methods.begin(); it != methods.end(); ++it) {
    FunctionScopePtr f = ImportFunctionScopePtr(ar, cls, *it);
    assert(!map[f->getName()]);
    map[f->getName()] = f;
  }
}

void BuiltinSymbols::ImportExtFunctions(AnalysisResultPtr ar,
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

    dest->add(pinfo->name.data(), typePtrFromDataType(pinfo->type),
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
    // And that if it's deferred (SID) it'll be a String.
    ClassInfo::ConstantInfo *cinfo = *it;
    dest->add(cinfo->name.data(),
              cinfo->isDeferred()
                ? (cinfo->isCallback() ? Type::Object : Type::String)
                : typePtrFromDataType(cinfo->getValue().getType()),
              ExpressionPtr(), ar, ConstructPtr());
  }
}

ClassScopePtr BuiltinSymbols::ImportClassScopePtr(AnalysisResultPtr ar,
                                                  ClassInfo *cls) {
  FunctionScopePtrVec methods;
  ImportExtFunctions(ar, methods, cls);

  ClassInfo::InterfaceVec ifaces = cls->getInterfacesVec();
  String parent = cls->getParentClass();
  std::vector<std::string> stdIfaces;
  if (!parent.empty() && (ifaces.empty() || ifaces[0] != parent)) {
    stdIfaces.push_back(parent.data());
  }
  for (auto it = ifaces.begin(); it != ifaces.end(); ++it) {
    stdIfaces.push_back(it->data());
  }

  ClassScopePtr cl(new ClassScope(ar, cls->getName().data(), parent.data(), stdIfaces, methods));
  for (uint i = 0; i < methods.size(); ++i) {
    methods[i]->setOuterScope(cl);
  }

  ImportExtProperties(ar, cl->getVariables(), cls);
  ImportExtConstants(ar, cl->getConstants(), cls);
  int attrs = cls->getAttribute();
  cl->setClassInfoAttribute(attrs);
  if (attrs & ClassInfo::HasDocComment) {
    cl->setDocComment(cls->getDocComment());
  }
  cl->setSystem();
  return cl;
}

void BuiltinSymbols::ImportExtClasses(AnalysisResultPtr ar) {
  const ClassInfo::ClassMap &classes = ClassInfo::GetClassesMap();
  for (auto it = classes.begin(); it != classes.end(); ++it) {
    ClassScopePtr cl = ImportClassScopePtr(ar, it->second);
    assert(!s_classes[cl->getName()]);
    s_classes[cl->getName()] = cl;
  }
}

void BuiltinSymbols::Parse(AnalysisResultPtr ar,
                           const std::string& phpBaseName,
                           const std::string& phpFileName) {
  const char *baseName = s_strings.add(phpBaseName.c_str());
  const char *fileName = s_strings.add(phpFileName.c_str());
  try {
    Scanner scanner(fileName, Option::ScannerType);
    Compiler::Parser parser(scanner, baseName, ar);
    if (!parser.parse()) {
      Logger::Error("Unable to parse file %s: %s", fileName,
                    parser.getMessage().c_str());
      assert(false);
    }
  } catch (FileOpenException &e) {
    Logger::Error("%s", e.getMessage().c_str());
  }
}

bool BuiltinSymbols::Load(AnalysisResultPtr ar, bool extOnly /* = false */) {
  if (Loaded) return true;
  Loaded = true;

  if (g_context.isNull()) init_thread_locals();
  ClassInfo::Load();

  // load extension functions first, so system/classes may call them
  ImportExtFunctions(ar, s_functions, ClassInfo::GetSystem());
  AnalysisResultPtr ar2 = AnalysisResultPtr(new AnalysisResult());
  s_variables = VariableTablePtr(new VariableTable(*ar2.get()));
  s_constants = ConstantTablePtr(new ConstantTable(*ar2.get()));

  // parse all PHP files under system/classes
  if (!extOnly) {
    ar = AnalysisResultPtr(new AnalysisResult());
    ar->loadBuiltinFunctions();
    string slib = systemlib_path();
    if (slib.empty()) {
      for (const char **cls = SystemClasses; *cls; cls++) {
        string phpBaseName = "/system/classes/";
        phpBaseName += *cls;
        phpBaseName += ".php";
        Parse(ar, phpBaseName, Option::GetSystemRoot() + phpBaseName);
      }
    } else {
      Parse(ar, slib, slib);
    }
    ar->analyzeProgram(true);
    ar->inferTypes();
    const StringToFileScopePtrMap &files = ar->getAllFiles();
    for (StringToFileScopePtrMap::const_iterator iterFile = files.begin();
         iterFile != files.end(); iterFile++) {
      const StringToClassScopePtrVecMap &classes =
        iterFile->second->getClasses();
      for (StringToClassScopePtrVecMap::const_iterator iter = classes.begin();
           iter != classes.end(); ++iter) {
        assert(iter->second.size() == 1);
        iter->second[0]->setSystem();
        assert(!s_classes[iter->first]);
        s_classes[iter->first] = iter->second[0];
      }
    }
  } else {
    NoSuperGlobals = true;
  }

  // load extension constants, classes and dynamics
  ImportExtConstants(ar, s_constants, ClassInfo::GetSystem());
  ImportExtClasses(ar);

  if (!extOnly) {
    Array constants = ClassInfo::GetSystemConstants();
    LocationPtr loc(new Location);
    for (ArrayIter it = constants.begin(); it; ++it) {
      CVarRef key = it.first();
      if (!key.isString()) continue;
      std::string name = key.toCStrRef().data();
      if (s_constants->getSymbol(name)) continue;
      if (name == "true" || name == "false" || name == "null") continue;
      CVarRef value = it.secondRef();
      if (!value.isInitialized() || value.isObject()) continue;
      ExpressionPtr e = Expression::MakeScalarExpression(ar2, ar2, loc, value);
      TypePtr t =
        value.isNull()    ? Type::Null    :
        value.isBoolean() ? Type::Boolean :
        value.isInteger() ? Type::Int64   :
        value.isDouble()  ? Type::Double  :
        value.isArray()   ? Type::Array   : Type::Variant;

      s_constants->add(key.toCStrRef().data(), t, e, ar2, e);
    }
    s_variables = ar2->getVariables();
    for (int i = 0, n = NumGlobalNames(); i < n; ++i) {
      s_variables->add(GlobalNames[i], Type::Variant, false, ar,
                       ConstructPtr(), ModifierExpressionPtr());
    }
  }
  s_constants->setDynamic(ar, "SID", true);

  return true;
}

AnalysisResultPtr BuiltinSymbols::LoadGlobalSymbols(const char *fileName) {
  AnalysisResultPtr ar(new AnalysisResult());
  string phpBaseName = "/system/globals/";
  phpBaseName += fileName;
  string phpFileName = Option::GetSystemRoot() + phpBaseName;
  const char *baseName = s_strings.add(phpBaseName.c_str());
  fileName = s_strings.add(phpFileName.c_str());

  try {
    Scanner scanner(fileName, Option::ScannerType);
    Compiler::Parser parser(scanner, baseName, ar);
    if (!parser.parse()) {
      assert(false);
      Logger::Error("Unable to parse file %s: %s", fileName,
                    parser.getMessage().c_str());
    }
  } catch (FileOpenException &e) {
    Logger::Error("%s", e.getMessage().c_str());
  }
  ar->analyzeProgram(true);
  ar->inferTypes();
  return ar;
}

void BuiltinSymbols::LoadFunctions(AnalysisResultPtr ar,
                                   StringToFunctionScopePtrMap &functions) {
  assert(Loaded);
  functions.insert(s_functions.begin(), s_functions.end());
}

void BuiltinSymbols::LoadClasses(AnalysisResultPtr ar,
                                 StringToClassScopePtrMap &classes) {
  assert(Loaded);
  classes.insert(s_classes.begin(), s_classes.end());
}

void BuiltinSymbols::LoadVariables(AnalysisResultPtr ar,
                                   VariableTablePtr variables) {
  assert(Loaded);
  if (s_variables) {
    variables->import(s_variables);
  }
}

void BuiltinSymbols::LoadConstants(AnalysisResultPtr ar,
                                   ConstantTablePtr constants) {
  assert(Loaded);
  if (s_constants) {
    constants->import(s_constants);
  }
}

ConstantTablePtr BuiltinSymbols::LoadSystemConstants() {
  AnalysisResultPtr ar = LoadGlobalSymbols("constants.php");
  const auto &fileScopes = ar->getAllFilesVector();
  if (!fileScopes.empty()) {
    return fileScopes[0]->getConstants();
  }
  throw std::runtime_error("LoadSystemConstants failed");
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
  if (NoSuperGlobals) return false;
  return s_superGlobals.find(name) != s_superGlobals.end();
}

TypePtr BuiltinSymbols::GetSuperGlobalType(const std::string &name) {
  StringToTypePtrMap::const_iterator iter = s_superGlobals.find(name);
  if (iter != s_superGlobals.end()) {
    return iter->second;
  }
  return TypePtr();
}
