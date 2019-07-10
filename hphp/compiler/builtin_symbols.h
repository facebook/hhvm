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

#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct AnalysisResult;
using AnalysisResultPtr = std::shared_ptr<AnalysisResult>;
struct UnitEmitter;

struct BuiltinSymbols {
  static AnalysisResultPtr s_systemAr;

  static void RecordSystemlibFile(std::unique_ptr<UnitEmitter>&&);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_BUILTIN_SYMBOLS_H_
