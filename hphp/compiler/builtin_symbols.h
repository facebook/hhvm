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

#ifndef incl_HPHP_BUILTIN_SYMBOLS_H_
#define incl_HPHP_BUILTIN_SYMBOLS_H_

#include "hphp/compiler/hphp.h"
#include <set>
#include "hphp/util/string-bag.h"
#include "hphp/runtime/base/class-info.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_EXTENDED_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_EXTENDED_BOOST_TYPES(FunctionScope);
DECLARE_EXTENDED_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(VariableTable);
DECLARE_BOOST_TYPES(ConstantTable);

class BuiltinSymbols {
public:
  static bool Loaded;
  static AnalysisResultPtr s_systemAr;

  static bool Load(AnalysisResultPtr ar);

  static void LoadFunctions(AnalysisResultPtr ar,
                            StringToFunctionScopePtrMap &functions);
  static void LoadClasses(AnalysisResultPtr ar,
                          StringToClassScopePtrMap &classes);
  static void LoadVariables(AnalysisResultPtr ar,
                            VariableTablePtr variables);
  static void LoadConstants(AnalysisResultPtr ar,
                            ConstantTablePtr constants);

  /**
   * Testing whether a variable is a PHP superglobal.
   */
  static bool IsSuperGlobal(const std::string &name);

  static bool IsDeclaredDynamic(const std::string& name);
  static void LoadSuperGlobals();

  static const char *const GlobalNames[];
  static int NumGlobalNames();
private:
  static StringBag s_strings;
  static const char *SystemClasses[];

  static hphp_string_set s_superGlobals;

  static std::set<std::string> s_declaredDynamic;

  static FunctionScopePtr ImportFunctionScopePtr(AnalysisResultPtr ar,
                                                 ClassInfo *cls,
                                                 ClassInfo::MethodInfo *method);
  static void ImportExtFunctions(AnalysisResultPtr ar,
                                 ClassInfo *cls);
  static void ImportExtMethods(AnalysisResultPtr ar,
                               FunctionScopePtrVec &vec,
                               ClassInfo *cls);
  static void ImportExtProperties(AnalysisResultPtr ar,
                                  VariableTablePtr dest,
                                  ClassInfo *cls);
  static void ImportExtConstants(AnalysisResultPtr ar,
                                 ConstantTablePtr dest,
                                 ClassInfo *cls);
  static void ImportNativeConstants(AnalysisResultPtr ar,
                                    ConstantTablePtr dest);
  static ClassScopePtr ImportClassScopePtr(AnalysisResultPtr ar,
                                           ClassInfo *cls);
  static void ImportExtClasses(AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_BUILTIN_SYMBOLS_H_
