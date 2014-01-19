/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/test/ext/test_base.h"
#include <sys/param.h>
#include "hphp/compiler/option.h"
#include "hphp/test/ext/test.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/ext/ext_array.h"

///////////////////////////////////////////////////////////////////////////////

char TestBase::error_buffer[MAXPATHLEN];

TestBase::TestBase() {
  Option::KeepStatementsWithNoEffect = true;
}

bool TestBase::Count(bool result) {
  if (result) {
    Test::s_passed++;
    pass_count++;
  } else {
    fail_count++;
  }

  Test::s_total++;
  return result;
}

bool TestBase::CountSkip() {
  skip_count++;
  Test::s_skipped++;
  Test::s_total++;
  return true;
}

bool TestBase::VerifySame(const char *exp1, const char *exp2,
                          CVarRef v1, CVarRef v2) {
  if (!same(v1, v2)) {
    g_context->obEndAll();
    printf("%s = \n", exp1); f_var_dump(v1);
    printf("%s = \n", exp2); f_var_dump(v2);
    return false;
  }
  return true;
}

bool TestBase::VerifyClose(const char *exp1, const char *exp2,
                           double v1, double v2) {
  double diff = v1 > v2 ? v1 - v2 : v2 - v1;
  if (diff > 0.00001) {
    g_context->obEndAll();
    printf("%s = \n", exp1); f_var_dump(v1);
    printf("%s = \n", exp2); f_var_dump(v2);
    return false;
  }
  return true;
}

bool TestBase::array_value_exists(CVarRef var, CVarRef value) {
  bool found = !same(f_array_search(value, var.toArray()), false);
  if (!found) {
    f_var_dump(var);
  }
  return found;
}
