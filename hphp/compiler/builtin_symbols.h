/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/util/hash-set.h"
#include <set>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_EXTENDED_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_EXTENDED_BOOST_TYPES(FunctionScope);
DECLARE_EXTENDED_BOOST_TYPES(ClassScope);

struct BuiltinSymbols {
  static bool Loaded;
  static AnalysisResultPtr s_systemAr;

  static bool Load(AnalysisResultPtr ar);

  /**
   * Testing whether a variable is a PHP superglobal.
   */
  static bool IsSuperGlobal(const std::string &name);

  static void LoadSuperGlobals();

  static const char *const GlobalNames[];
  static int NumGlobalNames();
private:
  static hphp_string_set s_superGlobals;

  static std::set<std::string> s_declaredDynamic;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_BUILTIN_SYMBOLS_H_
