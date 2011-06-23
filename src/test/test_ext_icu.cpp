/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <iostream>

///////////////////////////////////////////////////////////////////////////////

bool TestExtIcu::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_icu_transliterate);
  RUN_TEST(test_icu_tokenize);

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


bool TestExtIcu::test_icu_tokenize() {


  String input_eng = String("Hello World");
  Array output_eng = f_icu_tokenize(input_eng);

  VS(f_print_r(output_eng, true),
     "Array\n"
     "(\n"
     "    [0] => _B_\n"
     "    [1] => hello\n"
     "    [2] => world\n"
     "    [3] => _E_\n"
     ")\n"
    );
  String input_long = String("Hello! You are visitor #1234 to "
                            "http://www.facebook.com! "
                            "<3 How are you today (6/14/2011),"
                            " hello@world.com?");

  Array output_long = f_icu_tokenize(input_long);

  VS(f_print_r(output_long, true),
     "Array\n"
     "(\n"
     "    [0] => _B_\n"
     "    [1] => hello\n"
     "    [2] => !\n"
     "    [3] => you\n"
     "    [4] => are\n"
     "    [5] => visitor\n"
     "    [6] => #\n"
     "    [7] => XXXX\n"
     "    [8] => to\n"
     "    [9] => TOKEN_URL\n"
     "    [10] => !\n"
     "    [11] => TOKEN_HEART\n"
     "    [12] => how\n"
     "    [13] => are\n"
     "    [14] => you\n"
     "    [15] => today\n"
     "    [16] => (\n"
     "    [17] => TOKEN_DATE\n"
     "    [18] => )\n"
     "    [19] => ,\n"
     "    [20] => TOKEN_EMAIL\n"
     "    [21] => ?\n"
     "    [22] => _E_\n"
     ")\n"
    );

  String input_de = String("Ich mÃ¶chte Ã¼berzeugend oder Ã¤hnliche sein");
  Array output_de = f_icu_tokenize(input_de);

  VS(f_print_r(output_de, true),
     "Array\n"
     "(\n"
     "    [0] => _B_\n"
     "    [1] => ich\n"
     "    [2] => mã\n"
     "    [3] => ¶\n"
     "    [4] => chte\n"
     "    [5] => ã\n"
     "    [6] => ¼\n"
     "    [7] => berzeugend\n"
     "    [8] => oder\n"
     "    [9] => ã\n"
     "    [10] => ¤\n"
     "    [11] => hnliche\n"
     "    [12] => sein\n"
     "    [13] => _E_\n"
     ")\n");


  return Count(true);
}
