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
#include <runtime/eval/strict_mode.h>
#include <runtime/base/runtime_option.h>

namespace HPHP {

using namespace Eval;
using namespace std;
///////////////////////////////////////////////////////////////////////////////

void throw_strict(const ExtendedException &exn,
                  int strict_level_exn) {

  if (strict_level_exn <= RuntimeOption::StrictLevel) {
    if (RuntimeOption::StrictFatal) {
      throw exn;
    } else {
      int64 errnum = 2048LL; // E_STRICT
      raise_error_ex(exn.getMessage(), errnum, true, NeverThrow,
          "HipHop Strict warning:  ");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
