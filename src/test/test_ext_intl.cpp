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

#include <test/test_ext_intl.h>
#include <runtime/ext/ext_intl.h>

IMPLEMENT_SEP_EXTENSION_TEST(Intl);
///////////////////////////////////////////////////////////////////////////////

bool TestExtIntl::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_Normalizer);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIntl::test_Normalizer() {
  VERIFY(c_normalizer::ti_isnormalized(NULL, "\xC3\x85"));
  VERIFY(!c_normalizer::ti_isnormalized(NULL, "A\xCC\x8A"));
  VS(c_normalizer::ti_normalize(NULL, "A\xCC\x8A", q_normalizer_FORM_C),
     "\xC3\x85");
  return Count(true);
}
