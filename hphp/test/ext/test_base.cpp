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

#include "hphp/test/ext/test_base.h"
#include "hphp/compiler/option.h"
#include "hphp/test/ext/test.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/ext/array/ext_array.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"

///////////////////////////////////////////////////////////////////////////////

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
                          const Variant& v1, const Variant& v2) {
  if (!same(v1, v2)) {
    g_context->obEndAll();
    printf("%s = \n", exp1); HHVM_FN(var_dump)(v1);
    printf("%s = \n", exp2); HHVM_FN(var_dump)(v2);
    return false;
  }
  return true;
}

bool TestBase::VerifyClose(const char *exp1, const char *exp2,
                           double v1, double v2) {
  double diff = v1 > v2 ? v1 - v2 : v2 - v1;
  if (diff > 0.00001) {
    g_context->obEndAll();
    printf("%s = \n", exp1); HHVM_FN(var_dump)(v1);
    printf("%s = \n", exp2); HHVM_FN(var_dump)(v2);
    return false;
  }
  return true;
}

bool TestBase::array_value_exists(const Variant& var, const Variant& value) {
  bool found = !same(
    Variant::attach(HHVM_FN(array_search)(value, var.toArray())),
    false
  );
  if (!found) {
    HHVM_FN(var_dump)(var);
  }
  return found;
}
