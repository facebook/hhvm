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

#include <compiler/builtin_symbols.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/type.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/option.h>
#include <compiler/parser/parser.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/constant_table.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/base/class_info.h>
#include <util/logger.h>
#include <util/util.h>
#include <dlfcn.h>

using namespace HPHP;
using namespace std;
using namespace boost;

#define BF_COLUMN_COUNT  3
#define BF_COLUMN_NAME   0
#define BF_COLUMN_RETURN 1
#define BF_COLUMN_PARAMS 2

#define CLASS_TYPE 999

///////////////////////////////////////////////////////////////////////////////

bool BuiltinSymbols::Loaded = false;
bool BuiltinSymbols::NoSuperGlobals = false;
StringBag BuiltinSymbols::s_strings;

namespace HPHP {
#define EXT_TYPE 4
#include <system/ext.inc>
#undef EXT_TYPE
}

const char *BuiltinSymbols::ExtensionFunctions[] = {
#define S(n) (const char *)n
#define T(t) (const char *)Type::KindOf ## t
#define EXT_TYPE 0
#include <system/ext.inc>
  NULL,
};
#undef EXT_TYPE

const char *BuiltinSymbols::ExtensionConsts[] = {
#define EXT_TYPE 1
#include <system/ext.inc>
  NULL,
};
#undef EXT_TYPE

const char *BuiltinSymbols::ExtensionClasses[] = {
#define EXT_TYPE 2
#include <system/ext.inc>
  NULL,
};
#undef EXT_TYPE

const char *BuiltinSymbols::ExtensionDeclaredDynamic[] = {
#define EXT_TYPE 3
#include <system/ext.inc>
  NULL,
};
#undef EXT_TYPE

const char *BuiltinSymbols::HelperFunctions[] = {
#include <system/helper.inc>
  NULL,
};

StringToFunctionScopePtrMap BuiltinSymbols::s_functions;
StringToFunctionScopePtrMap BuiltinSymbols::s_helperFunctions;

const char *BuiltinSymbols::SystemClasses[] = {
  "stdclass",
  "exception",
  "arrayaccess",
  "iterator",
  "closure",
  "reflection",
  "splobjectstorage",
  "directory",
  "splfile",
  "debugger",
  "xhprof",
  NULL
};

const char *BuiltinSymbols::BaseSysRsrcClasses[] = {
  "SplFileInfo",
  "RecursiveIteratorIterator",
  NULL
};

StringToClassScopePtrMap BuiltinSymbols::s_classes;
VariableTablePtr BuiltinSymbols::s_variables;
ConstantTablePtr BuiltinSymbols::s_constants;
StringToTypePtrMap BuiltinSymbols::s_superGlobals;
std::set<std::string> BuiltinSymbols::s_declaredDynamic;
void *BuiltinSymbols::s_handle_main = NULL;

///////////////////////////////////////////////////////////////////////////////

void BuiltinSymbols::ParseExtFunctions(AnalysisResultPtr ar, const char **p,
                                       bool sep) {
  while (*p) {
    FunctionScopePtr f = ParseExtFunction(ar, p);
    if (sep) {
      f->setSepExtension();
    }
    ASSERT(!s_functions[f->getName()]);
    s_functions[f->getName()] = f;
  }
}

void BuiltinSymbols::ParseExtConsts(AnalysisResultPtr ar, const char **p,
                                    bool sep) {
  while (*p) {
    const char *name = *p++;
    TypePtr type = ParseType(p);
    s_constants->add(name, type, ExpressionPtr(), ar, ConstructPtr());
    if (sep) {
      s_constants->setSepExtension(name);
    }
  }
}

TypePtr BuiltinSymbols::ParseType(const char **&p) {
  const char *clsname = NULL;
  Type::KindOf ktype = (Type::KindOf)(long)(*p++);
  if (ktype == CLASS_TYPE) {
    clsname = *p++;
  }
  TypePtr type;
  if (clsname) {
    type = Type::CreateObjectType(clsname);
  } else if (ktype != Type::KindOfVoid) {
    type = Type::GetType(ktype);
  }
  return type;
}

void BuiltinSymbols::ParseExtClasses(AnalysisResultPtr ar, const char **p,
                                     bool sep) {
  while (*p) {
    // Parse name
    const char *cname = *p++;
    // Parse parent
    const char *cparent = *p++;
    if (!cparent) cparent = "";
    // Parse list of interfaces
    vector<string> ifaces;
    while (*p) ifaces.push_back(*p++);
    p++;
    // Parse methods
    FunctionScopePtrVec methods;
    while (*p) {
      FunctionScopePtr fs = ParseExtFunction(ar, p, true);
      if (sep) {
        fs->setSepExtension();
      }
      int flags = (int)(int64)(*p++);
      if (flags & ClassInfo::IsAbstract) {
        fs->addModifier(T_ABSTRACT);
      }
      int vismod = 0;
      if (flags & ClassInfo::IsProtected) {
        vismod = T_PROTECTED;
      } else if (flags & ClassInfo::IsPrivate) {
        vismod = T_PRIVATE;
      }
      fs->addModifier(vismod);
      if (flags & ClassInfo::IsStatic) {
        fs->addModifier(T_STATIC);
      }
      methods.push_back(fs);
    }
    if (cparent && *cparent && (ifaces.empty() || ifaces[0] != cparent)) {
      ifaces.insert(ifaces.begin(), cparent);
    }
    ClassScopePtr cl(new ClassScope(ar, cname, cparent, ifaces, methods));
    for (uint i = 0; i < methods.size(); ++i) {
      methods[i]->setOuterScope(cl);
    }
    p++;
    // Parse properties
    while (*p) {
      p++; // TODO, support visibility
      const char *name = *p++;
      TypePtr type = ParseType(p);
      cl->getVariables()->add(name, type, false, ar, ExpressionPtr(),
                              ModifierExpressionPtr());
    }
    p++;
    // Parse consts
    while (*p) {
      const char *name = *p++;
      TypePtr type = ParseType(p);
      cl->getConstants()->add(name, type, ExpressionPtr(), ar, ConstructPtr());
    }
    p++;

    int flags = (int)(int64)(*p++);
    cl->setClassInfoAttribute(flags);
    if (flags & ClassInfo::HasDocComment) {
      cl->setDocComment(*p++);
    }

    cl->setSystem();
    if (sep) {
      cl->setSepExtension();
    }
    s_classes[cl->getName()] = cl;
  }
}

void BuiltinSymbols::ParseExtDynamics(AnalysisResultPtr ar, const char **p,
                                      bool sep) {
  while (*p) {
    s_declaredDynamic.insert(Util::toLower(string(*p)));
    p++;
  }
}

FunctionScopePtr BuiltinSymbols::ParseExtFunction(AnalysisResultPtr ar,
    const char** &p, bool method /* = false */) {
  const char *name = *p++;
  TypePtr retType = ParseType(p);
  bool reference = *p++;

  int minParam = -1;
  int maxParam = 0;
  const char **arg = p;
  while (*arg) {
    /* name */ arg++;
    ParseType(arg);
    const char *argDefault = *arg++;
    /* const char *argDefaultText = */ arg++;
    /* bool argReference = */ arg++;
    if (argDefault && minParam < 0) {
      minParam = maxParam;
    }
    maxParam++;
  }
  if (minParam < 0) minParam = maxParam;

  FunctionScopePtr f(new FunctionScope(method, name, reference));
  f->setParamCounts(ar, minParam, maxParam);
  if (retType) {
    f->setReturnType(ar, retType);
  }

  int index = 0;
  const char *paramName = NULL;
  while ((paramName = *p++ /* argName */)) {
    TypePtr argType = ParseType(p);
    const char *argDefault = *p++;
    const char *argDefaultText = *p++;
    bool argReference = *p++;

    f->setParamName(index, paramName);
    if (argReference) f->setRefParam(index);
    f->setParamType(ar, index, argType);
    if (argDefault) f->setParamDefault(index, argDefault, argDefaultText);

    index++;
  }

  int flags = (int)(int64)(*p++);
  f->setClassInfoAttribute(flags);
  if (flags & ClassInfo::HasDocComment) {
    f->setDocComment(*p++);
  }
  if (flags & ClassInfo::HasOptFunction) {
    f->setOptFunction((FunctionOptPtr)(*p++));
  }

  // This block of code is not needed, if BlockScope directly takes flags.
  if (flags & ClassInfo::MixedVariableArguments) {
    f->setVariableArgument(-1);
  } else if (flags & ClassInfo::RefVariableArguments) {
    f->setVariableArgument(1);
  } else if (flags & ClassInfo::VariableArguments) {
    f->setVariableArgument(0);
  }
  if (flags & ClassInfo::NoEffect) {
    f->setNoEffect();
  }
  if (flags & ClassInfo::FunctionIsFoldable) {
    f->setIsFoldable();
  }
  if (flags & ClassInfo::ContextSensitive) {
    f->setContextSensitive(true);
  }
  return f;
}

FunctionScopePtr BuiltinSymbols::ParseHelperFunction(AnalysisResultPtr ar,
                                                     const char** &p) {
  FunctionScopePtr f = ParseExtFunction(ar, p);
  f->setHelperFunction();
  return f;
}

bool BuiltinSymbols::LoadSepExtensionSymbols(AnalysisResultPtr ar,
                                             const std::string &name,
                                             const std::string &soname) {
  string mapname = name + "_map";

  const char ***symbols = NULL;

  // If we linked with .a, the symbol is already in main program.
  if (s_handle_main == NULL) {
    s_handle_main = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
    if (!s_handle_main) {
      const char *error = dlerror();
      Logger::Error("Unable to load main program's symbols: %s",
                    error ? error : "(unknown)");
    }
  }
  if (s_handle_main) {
    symbols = (const char ***)dlsym(s_handle_main, mapname.c_str());
  }

  // Otherwise, look for .so to load it.
  void *handle = NULL;
  if (!symbols) {
    handle = dlopen(soname.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
      const char *error = dlerror();
      Logger::Error("Unable to load %s: %s", soname.c_str(),
                    error ? error : "(unknown)");
      return false;
    }
    symbols = (const char ***)dlsym(handle, mapname.c_str());
    if (!symbols) {
      Logger::Error("Unable to find %s in %s", mapname.c_str(),
                    soname.c_str());
      dlclose(handle);
      return false;
    }
  }

  ParseExtFunctions(ar, symbols[0], true);
  ParseExtConsts   (ar, symbols[1], true);
  ParseExtClasses  (ar, symbols[2], true);
  ParseExtDynamics (ar, symbols[3], true);

  if (handle) {
    /*
      Not closing for now, because it may have set an object allocator,
      which would then fail next time its used.
      I think the object allocators should be fixed instead - one per size,
      rather than one per class, then this issue wouldnt occur
    */
    // dlclose(handle);
  }
  return true;
}

bool BuiltinSymbols::Load(AnalysisResultPtr ar, bool extOnly /* = false */) {
  if (Loaded) return true;
  Loaded = true;

  // Build function scopes for some of the runtime helper functions
  // declared in "runtime/base/builtin_functions.h"
  const char **helper = HelperFunctions;
  while (*helper) {
    FunctionScopePtr f = ParseHelperFunction(ar, helper);
    ASSERT(!s_helperFunctions[f->getName()]);
    s_helperFunctions[f->getName()] = f;
  }

  // load extension functions first, so system/classes may call them
  ParseExtFunctions(ar, ExtensionFunctions, false);
  AnalysisResultPtr ar2;

  // parse all PHP files under system/classes
  if (!extOnly) {
    ar = AnalysisResultPtr(new AnalysisResult());
    ar->loadBuiltinFunctions();
    for (const char **cls = SystemClasses; *cls; cls++) {
      string phpBaseName = "/system/classes/";
      phpBaseName += *cls;
      phpBaseName += ".php";
      string phpFileName = Option::GetSystemRoot() + phpBaseName;
      const char *baseName = s_strings.add(phpBaseName.c_str());
      const char *fileName = s_strings.add(phpFileName.c_str());
      try {
        Scanner scanner(fileName, Option::ScannerType);
        Compiler::Parser parser(scanner, baseName, ar);
        if (!parser.parse()) {
          Logger::Error("Unable to parse file %s: %s", fileName,
                        parser.getMessage().c_str());
          ASSERT(false);
        }
      } catch (FileOpenException &e) {
        Logger::Error("%s", e.getMessage().c_str());
      }
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
        ASSERT(iter->second.size() == 1);
        iter->second[0]->setSystem();
        ASSERT(!s_classes[iter->first]);
        s_classes[iter->first] = iter->second[0];
      }
    }

    // parse globals/variables.php and globals/constants.php
    NoSuperGlobals = true;
    s_variables = LoadGlobalSymbols("symbols.php")->getVariables();
    ar2 = LoadGlobalSymbols("constants.php");
    const FileScopePtrVec &fileScopes = ar2->getAllFilesVector();
    if (!fileScopes.empty()) {
      s_constants = fileScopes[0]->getConstants();
    } else {
      ar2 = AnalysisResultPtr(new AnalysisResult());
      s_constants = ConstantTablePtr(new ConstantTable(*ar2.get()));
    }
    NoSuperGlobals = false;
  } else {
    ar2 = AnalysisResultPtr(new AnalysisResult());
    s_variables = VariableTablePtr(new VariableTable(*ar2.get()));
    s_constants = ConstantTablePtr(new ConstantTable(*ar2.get()));
    NoSuperGlobals = true;
  }
  s_constants->setDynamic(ar, "SID");

  // load extension constants, classes and dynamics
  ParseExtConsts(ar, ExtensionConsts, false);
  ParseExtClasses(ar, ExtensionClasses, false);
  ParseExtDynamics(ar, ExtensionDeclaredDynamic, false);
  for (unsigned int i = 0; i < Option::SepExtensions.size(); i++) {
    Option::SepExtensionOptions &options = Option::SepExtensions[i];
    string soname = options.soname;
    if (soname.empty()) {
      soname = string("lib") + options.name + ".so";
    }
    if (!options.lib_path.empty()) {
      soname = options.lib_path + "/" + soname;
    }
    if (!LoadSepExtensionSymbols(ar, options.name, soname)) {
      return false;
    }
  }

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
      ASSERT(false);
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
                                   StringToFunctionScopePtrVecMap &functions) {
  ASSERT(Loaded);
  for (StringToFunctionScopePtrMap::const_iterator it = s_functions.begin();
       it != s_functions.end(); ++it) {
    if (functions.find(it->first) == functions.end()) {
      functions[it->first].push_back(it->second);
      FunctionScope::RecordFunctionInfo(it->first, it->second);
    }
  }
}

void BuiltinSymbols::LoadHelperFunctions(
  AnalysisResultPtr ar,
  StringToFunctionScopePtrVecMap &functions) {
  ASSERT(Loaded);
  for (StringToFunctionScopePtrMap::const_iterator it =
          s_helperFunctions.begin(); it != s_helperFunctions.end(); ++it) {
    if (functions.find(it->first) == functions.end()) {
      functions[it->first].push_back(it->second);
    }
  }
}

void BuiltinSymbols::LoadClasses(AnalysisResultPtr ar,
                                 StringToClassScopePtrMap &classes) {
  ASSERT(Loaded);
  classes.insert(s_classes.begin(), s_classes.end());

  // we are adding these builtin functions, so that user-defined functions
  // will not overwrite them with their own file and line number information
  for (StringToClassScopePtrMap::const_iterator iter =
         s_classes.begin(); iter != s_classes.end(); ++iter) {
    const StringToFunctionScopePtrVecMap &funcs = iter->second->getFunctions();
    for (StringToFunctionScopePtrVecMap::const_iterator iter =
           funcs.begin(); iter != funcs.end(); ++iter) {
      FunctionScope::RecordFunctionInfo(iter->first, iter->second.back());
    }
  }
}

void BuiltinSymbols::LoadVariables(AnalysisResultPtr ar,
                                   VariableTablePtr variables) {
  ASSERT(Loaded);
  if (s_variables) {
    variables->import(s_variables);
  }
}

void BuiltinSymbols::LoadConstants(AnalysisResultPtr ar,
                                   ConstantTablePtr constants) {
  ASSERT(Loaded);
  if (s_constants) {
    constants->import(s_constants);
  }
}

void BuiltinSymbols::LoadBaseSysRsrcClasses(AnalysisResultPtr ar,
                                            set<string> &baseSysRsrcClasses) {
  ASSERT(Loaded);
  for (const char **cls = BaseSysRsrcClasses; *cls; cls++) {
    baseSysRsrcClasses.insert(*cls);
  }
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
  LoadSuperGlobals();
  return s_superGlobals.find(name) != s_superGlobals.end();
}

TypePtr BuiltinSymbols::GetSuperGlobalType(const std::string &name) {
  LoadSuperGlobals();
  StringToTypePtrMap::const_iterator iter = s_superGlobals.find(name);
  if (iter != s_superGlobals.end()) {
    return iter->second;
  }
  return TypePtr();
}

bool BuiltinSymbols::IsDeclaredDynamic(const std::string& name) {
  return s_declaredDynamic.find(name) != s_declaredDynamic.end();
}
