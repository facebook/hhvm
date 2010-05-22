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

#ifndef __BUILTIN_SYMBOLS_H__
#define __BUILTIN_SYMBOLS_H__

#include <compiler/hphp.h>
#include <util/string_bag.h>

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
                            StringToFunctionScopePtrVecMap &functions);
  static void LoadHelperFunctions(AnalysisResultPtr ar,
                                  StringToFunctionScopePtrVecMap &functions);
  static void LoadClasses(AnalysisResultPtr ar,
                          StringToClassScopePtrMap &classes);
  static void LoadVariables(AnalysisResultPtr ar,
                            VariableTablePtr variables);
  static void LoadConstants(AnalysisResultPtr ar,
                            ConstantTablePtr constants);
  static void LoadBaseSysRsrcClasses(AnalysisResultPtr ar,
                                     std::set<std::string> &baseSysRsrcClasses);

  /**
   * Testing whether a variable is a PHP superglobal.
   */
  static bool IsSuperGlobal(const std::string &name);
  static TypePtr GetSuperGlobalType(const std::string &name);

  static bool IsDeclaredDynamic(const std::string& name);

  static StringToFunctionScopePtrMap s_functions;
  static StringToFunctionScopePtrMap s_helperFunctions;
  static StringToClassScopePtrMap s_classes;
  static VariableTablePtr s_variables;
  static ConstantTablePtr s_constants;

private:
  static StringBag s_strings;
  static const char *ExtensionFunctions[];
  static const char *ExtensionClasses[];
  static const char *ExtensionConsts[];
  static const char *ExtensionDeclaredDynamic[];
  static const char *SystemClasses[];
  static const char *BaseSysRsrcClasses[];
  static const char *HelperFunctions[];
  static AnalysisResultPtr LoadGlobalSymbols(const char *fileName);

  static StringToTypePtrMap s_superGlobals;
  static void LoadSuperGlobals();

  static std::set<std::string> s_declaredDynamic;

  static void *s_handle_main;
  static bool LoadSepExtensionSymbols(AnalysisResultPtr ar,
                                      const std::string &name,
                                      const std::string &soname);

  static void ParseExtFunctions(AnalysisResultPtr ar, const char **p,
                                bool sep);
  static void ParseExtConsts(AnalysisResultPtr ar, const char **p, bool sep);
  static void ParseExtClasses(AnalysisResultPtr ar, const char **p, bool sep);
  static void ParseExtDynamics(AnalysisResultPtr ar, const char **p, bool sep);

  static FunctionScopePtr ParseExtFunction(AnalysisResultPtr ar,
                                           const char** &p);
  static FunctionScopePtr ParseHelperFunction(AnalysisResultPtr ar,
                                              const char** &p);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __BUILTIN_SYMBOLS_H__
