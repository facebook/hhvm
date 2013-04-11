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

#ifndef __BUILTIN_SYMBOLS_H__
#define __BUILTIN_SYMBOLS_H__

#include <compiler/hphp.h>
#include <util/string_bag.h>
#include <runtime/base/class_info.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(VariableTable);
DECLARE_BOOST_TYPES(ConstantTable);

class BuiltinSymbols {
public:
  static bool Loaded;
  static bool NoSuperGlobals; // for SystemCPP bootstraping only

  static bool Load(AnalysisResultPtr ar, bool extOnly = false);

  static void LoadFunctions(AnalysisResultPtr ar,
                            StringToFunctionScopePtrMap &functions);
  static void LoadClasses(AnalysisResultPtr ar,
                          StringToClassScopePtrMap &classes);
  static void LoadVariables(AnalysisResultPtr ar,
                            VariableTablePtr variables);
  static void LoadConstants(AnalysisResultPtr ar,
                            ConstantTablePtr constants);

  /*
   * Load system/globals/constants.php.
   */
  static ConstantTablePtr LoadSystemConstants();

  /**
   * Testing whether a variable is a PHP superglobal.
   */
  static bool IsSuperGlobal(const std::string &name);
  static TypePtr GetSuperGlobalType(const std::string &name);

  static bool IsDeclaredDynamic(const std::string& name);
  static void LoadSuperGlobals();

  static StringToFunctionScopePtrMap s_functions;
  static StringToClassScopePtrMap s_classes;
  static VariableTablePtr s_variables;
  static ConstantTablePtr s_constants;

  static const char *const GlobalNames[];
  static int NumGlobalNames();
private:
  static StringBag s_strings;
  static const char *SystemClasses[];

  static AnalysisResultPtr LoadGlobalSymbols(const char *fileName);
  static void Parse(AnalysisResultPtr ar,
                    const std::string& phpBaseName,
                    const std::string& phpFileName);

  static StringToTypePtrMap s_superGlobals;

  static std::set<std::string> s_declaredDynamic;

  static void *s_handle_main;

  static FunctionScopePtr ImportFunctionScopePtr(AnalysisResultPtr ar,
                                                 ClassInfo *cls,
                                                 ClassInfo::MethodInfo *method);
  static void ImportExtFunctions(AnalysisResultPtr ar,
                                 StringToFunctionScopePtrMap &map,
                                 ClassInfo *cls);
  static void ImportExtFunctions(AnalysisResultPtr ar,
                                 FunctionScopePtrVec &vec,
                                 ClassInfo *cls);
  static void ImportExtProperties(AnalysisResultPtr ar,
                                  VariableTablePtr dest,
                                  ClassInfo *cls);
  static void ImportExtConstants(AnalysisResultPtr ar,
                                 ConstantTablePtr dest,
                                 ClassInfo *cls);
  static ClassScopePtr ImportClassScopePtr(AnalysisResultPtr ar,
                                           ClassInfo *cls);
  static void ImportExtClasses(AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __BUILTIN_SYMBOLS_H__
