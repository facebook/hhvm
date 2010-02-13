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

#include <lib/hphp.h>
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

  static void load(AnalysisResultPtr ar, bool extOnly = false);

  static void loadFunctions(AnalysisResultPtr ar,
                            StringToFunctionScopePtrVecMap &functions);
  static void loadHelperFunctions(AnalysisResultPtr ar,
                                  StringToFunctionScopePtrVecMap &functions);
  static void loadClasses(AnalysisResultPtr ar,
                          StringToClassScopePtrMap &classes);
  static void loadVariables(AnalysisResultPtr ar,
                            VariableTablePtr variables);
  static void loadConstants(AnalysisResultPtr ar,
                            ConstantTablePtr constants);
  static void loadBaseSysRsrcClasses(AnalysisResultPtr ar,
                                     std::set<std::string> &baseSysRsrcClasses);

  /**
   * Testing whether a variable is a PHP superglobal.
   */
  static bool isSuperGlobal(const std::string &name);
  static TypePtr getSuperGlobalType(const std::string &name);

  static bool isDeclaredDynamic(const std::string& name);

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
  static AnalysisResultPtr loadGlobalSymbols(const char *fileName);

  static StringToTypePtrMap s_superGlobals;
  static void loadSuperGlobals();

  static std::set<std::string> s_declaredDynamic;

  static void parseExtConsts(AnalysisResultPtr ar);
  static void parseExtClasses(AnalysisResultPtr ar);
  static FunctionScopePtr parseExtFunction(AnalysisResultPtr ar,
                                           const char** &p);
  static FunctionScopePtr parseHelperFunction(AnalysisResultPtr ar,
                                              const char** &p);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __BUILTIN_SYMBOLS_H__
