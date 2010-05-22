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

#include <test/test_ext_icu.h>
#include <runtime/ext/ext_icu.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtIcu::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_icu_transliterate);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

// Test string lifted from tests/intl/utf8.h
bool TestExtIcu::test_icu_transliterate() {
  String input_ru =
    String("\xd1\x84\xd0\xb5\xd0\xb9\xd1"
           "\x81\xd0\xb1\xd1\x83\xc5\x93\xd0\xba",
           CopyString);
  String output_ru = f_icu_transliterate(input_ru, false);
  // Note: different than php test ('y' -> 'j')
  VERIFY(output_ru == "fejsbu\xc5\x93k");

  // Verify that removing accents works.
  String input_de = String("Ich m\xc3\xb6"
                           "chte \xc3\xbc"
                           "berzeugend "
                            "oder \xc3\xa4hnliche sein",
                           CopyString);
  String output_de = f_icu_transliterate(input_de, true);
  VERIFY(output_de == "Ich mochte uberzeugend oder ahnliche sein");

  // Verify that keeping accents works.
  VERIFY(f_icu_transliterate(input_de, false) == (const char*)input_de);

  // Check an non-Latin language.
  String input_zh = String("\xe5\x9b\x9b"
                           "\xe5\x8d\x81\xe5\x9b\x9b\xe7"
                           "\x9f\xb3\xe7\x8d\x85\xe5\xad\x90",
                           CopyString);
  String output_zh = f_icu_transliterate(input_zh, true);
  VERIFY(output_zh == "si shi si shi shi zi");

  return Count(true);
}
