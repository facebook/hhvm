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
#include <compiler/analysis/dependency_graph.h>
#include <compiler/parser/hphp.tab.hpp>
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

///////////////////////////////////////////////////////////////////////////////

bool BuiltinSymbols::Loaded = false;
bool BuiltinSymbols::NoSuperGlobals = false;
StringBag BuiltinSymbols::s_strings;

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
  "pear_error",
  "reflection",
  "splobjectstorage",
  "directory",
  "splfile",
  NULL
};

const char *BuiltinSymbols::BaseSysRsrcClasses[] = {
  "splfileinfo",
  "recursiveiteratoriterator",
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
    // Parse name
    const char *name = *p++;
    // Parse type
    Type::KindOf type = (Type::KindOf)(long)(*p++);
    s_constants->add(name, Type::GetType(type),
                     ExpressionPtr(), ar, ConstructPtr());
    if (sep) {
      s_constants->setSepExtension(name);
    }
  }
}

void BuiltinSymbols::ParseExtClasses(AnalysisResultPtr ar, const char **p,
                                     bool sep) {
  while (*p) {
    // Parse name
    const char *cname = *p++;
    // Parse parent
    const char *cparent = *p++;
    // Parse list of interfaces
    vector<string> ifaces;
    while (*p) ifaces.push_back(*p++);
    p++;
    // Parse methods
    vector<FunctionScopePtr> methods;
    while (*p) {
      FunctionScopePtr fs = ParseExtFunction(ar, p);
      if (sep) {
        fs->setSepExtension();
      }
      bool abstract = (bool)*p++;
      if (abstract) {
        fs->addModifier(T_ABSTRACT);
      }
      int visibility = (long)*p++;
      int vismod = 0;
      if (visibility == 1) {
        vismod = T_PROTECTED;
      } else if (visibility == 2) {
        vismod = T_PRIVATE;
      }
      fs->addModifier(vismod);
      bool stat = (bool)*p++;
      if (stat) {
        fs->addModifier(T_STATIC);
      }
      methods.push_back(fs);
    }
    if (cparent && *cparent && (ifaces.empty() || ifaces[0] != cparent)) {
      ifaces.insert(ifaces.begin(), cparent);
    }
    ClassScopePtr cl(new ClassScope(ar, string(cname), string(cparent),
                                    ifaces, methods));
    p++;
    // Parse consts
    while (*p) {
      const char *name = *p++;
      // Parse type
      Type::KindOf type = (Type::KindOf)(long)(*p++);
      cl->getConstants()->add(name, Type::GetType(type),
                              ExpressionPtr(), ar, ConstructPtr());
    }
    p++;
    cl->setSystem();
    if (sep) {
      cl->setSepExtension();
    }
    s_classes[cname] = cl;
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
                                                  const char** &p) {
  const char *name = *p++;
  Type::KindOf retType = (Type::KindOf)(long)(*p++);
  bool reference = *p++;

  int minParam = -1;
  int maxParam = 0;
  const char **arg = p;
  while (*arg) {
    /* const char *argName = */ arg++;
    /* Type::KindOf argType = (Type::KindOf) */ arg++;
    const char *argDefault = *arg++;
    /* bool argReference = */ arg++;
    if (argDefault && minParam < 0) {
      minParam = maxParam;
    }
    maxParam++;
  }
  if (minParam < 0) minParam = maxParam;

  string lowered = Util::toLower(name);
  FunctionScopePtr f(new FunctionScope(false, lowered, reference));
  f->setParamCounts(minParam, maxParam);
  if (retType != Type::KindOfVoid) {
    f->setReturnType(ar, Type::GetType(retType));
  }

  int index = 0;
  const char *paramName = NULL;
  while ((paramName = *p++ /* argName */)) {
    Type::KindOf argType = (Type::KindOf)(long)*p++;
    /* const char *argDefault = */ p++;
    bool argReference = *p++;

    f->setParamName(index, paramName);
    if (argReference) f->setRefParam(index);
    f->setParamType(ar, index, Type::GetType(argType));

    index++;
  }

  // Read function flags (these flags are defined in "idl/base.php")
  int flags = (int)(int64)(*p++);
  // Flags for variable arguments
  if (flags & 0x2) {
    f->setVariableArgument(true);
  } else if (flags & 0x1) {
    f->setVariableArgument(false);
  }
  // Flag for no side effects
  if (flags & 0x4) {
    f->setNoEffect();
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
    dlclose(handle);
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

  // parse all PHP files under system/classes
  if (!extOnly) {
    ar = AnalysisResultPtr(new AnalysisResult());
    for (const char **cls = SystemClasses; *cls; cls++) {
      string phpBaseName = "/system/classes/";
      phpBaseName += *cls;
      phpBaseName += ".php";
      string phpFileName = Option::GetSystemRoot() + phpBaseName;
      const char *baseName = s_strings.add(phpBaseName.c_str());
      const char *fileName = s_strings.add(phpFileName.c_str());
      try {
        Scanner scanner(new ylmm::basic_buffer(fileName), true, false);
        ParserPtr parser(new Parser(scanner, baseName, 0, ar));
        if (parser->parse()) {
          Logger::Error("Unable to parse file %s: %s", fileName,
                        parser->getMessage().c_str());
          ASSERT(false);
        }
      } catch (std::runtime_error) {
        Logger::Error("Unable to open file %s", fileName);
      }
    }
    ar->analyzeProgram();
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
    const std::vector<FileScopePtr> &fileScopes =
      LoadGlobalSymbols("constants.php")->getAllFilesVector();
    if (!fileScopes.empty()) {
      s_constants = fileScopes[0]->getConstants();
    } else {
      AnalysisResult ar2;
      s_constants = ConstantTablePtr(new ConstantTable(ar2));
    }
    NoSuperGlobals = false;
  } else {
    AnalysisResult ar2;
    s_variables = VariableTablePtr(new VariableTable(ar2));
    s_constants = ConstantTablePtr(new ConstantTable(ar2));
    NoSuperGlobals = true;
  }

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
    Scanner scanner(new ylmm::basic_buffer(fileName), true, false);
    ParserPtr parser(new Parser(scanner, baseName, 0, ar));
    if (parser->parse()) {
      ASSERT(false);
      Logger::Error("Unable to parse file %s: %s", fileName,
                    parser->getMessage().c_str());
    }
  } catch (std::runtime_error) {
    Logger::Error("Unable to open file %s", fileName);
  }
  ar->analyzeProgram();
  ar->inferTypes();
  return ar;
}

void BuiltinSymbols::LoadFunctions(AnalysisResultPtr ar,
                                   StringToFunctionScopePtrVecMap &functions) {
  ASSERT(Loaded);
  for (StringToFunctionScopePtrMap::const_iterator it = s_functions.begin();
       it != s_functions.end(); ++it) {
    functions[it->first].push_back(it->second);
  }

  // we are adding these builtin functions, so that user-defined functions
  // will not overwrite them with their own file and line number information
  for (StringToFunctionScopePtrMap::const_iterator iter =
         s_functions.begin(); iter != s_functions.end(); ++iter) {
    ar->getDependencyGraph()->addParent(DependencyGraph::KindOfFunctionCall,
                                        "", iter->first, StatementPtr());
  }
}

void BuiltinSymbols::LoadHelperFunctions(AnalysisResultPtr ar,
                                         StringToFunctionScopePtrVecMap &functions) {
  ASSERT(Loaded);
  for (StringToFunctionScopePtrMap::const_iterator it =
          s_helperFunctions.begin(); it != s_helperFunctions.end(); ++it) {
    functions[it->first].push_back(it->second);
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
    ar->getDependencyGraph()->addParent(DependencyGraph::KindOfClassDerivation,
                                        "", iter->first, StatementPtr());
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
    s_superGlobals["argc"] = Type::Variant;
    s_superGlobals["argv"] = Type::Variant;

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
