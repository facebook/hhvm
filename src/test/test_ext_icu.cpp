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

  RUN_TEST(test_icu_match);
  RUN_TEST(test_icu_transliterate);
  RUN_TEST(test_icu_tokenize);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIcu::test_icu_match() {
  // Test subject strings.
  String subject = String(
    "\u05d6\U00010905 PHP is a scripting language. \ufeb0\ufef3",
    CopyString);
  String subject_32 = String(
    "\U00010905\U00010905\U00010905\U00010905\U00010905\U00010905",
    CopyString);
  String subject_en = String("this is an english string", CopyString);
  // "this is a hebrew string"
  String subject_he = String(
    "\u05d6\u05d4 \u05d4\u05d5\u05d0 \u05de\u05d7\u05e8\u05d5\u05d6\u05ea "
    "\u05e2\u05d1\u05e8\u05d9\u05ea",
    CopyString);
  // "this is an arabic string"
  String subject_ar = String(
    "\ufee9\ufeab\ufe8d \ufee9\ufeed \ufe8e\ufee0\ufee8\ufebb "
    "\ufe8d\ufefa\ufee8\ufea0\ufee0\ufef3\ufeb0\ufef3",
    CopyString);
  // "this is a hebrew string"
  String subject_mixed = String(
    "this is a \u05e2\u05d1\u05e8\u05d9\u05ea string",
    CopyString);

  // Test basic regex parsing functionality.
  VERIFY(f_icu_match("scripting", subject));
  VERIFY(!f_icu_match("php", subject));
  VERIFY(f_icu_match("(\\bPHP\\b)", subject));
  VERIFY(!f_icu_match("(\\bPHP\\b))", subject));

  // Test returning matches functionality.
  Variant matches;
  VERIFY(f_icu_match("(PHP) is", subject, ref(matches)));
  VS(f_print_r(matches, true),
    "Array\n"
    "(\n"
    "    [0] => PHP is\n"
    "    [1] => PHP\n"
    ")\n");
  VERIFY(f_icu_match("is (a)", subject, ref(matches),
                     k_UREGEX_OFFSET_CAPTURE));
  VS(f_print_r(matches, true),
     "Array\n"
     "(\n"
     "    [0] => Array\n"
     "        (\n"
     "            [0] => is a\n"
     "            [1] => 7\n"
     "        )\n"
     "\n"
     "    [1] => Array\n"
     "        (\n"
     "            [0] => a\n"
     "            [1] => 10\n"
     "        )\n"
     "\n"
     ")\n");
  VERIFY(f_icu_match("\\. \ufeb0", subject, ref(matches),
                     k_UREGEX_OFFSET_CAPTURE));
  VS(f_print_r(matches, true),
    "Array\n"
    "(\n"
    "    [0] => Array\n"
    "        (\n"
    "            [0] => . \ufeb0\n"
    "            [1] => 30\n"
    "        )\n"
    "\n"
    ")\n");
  VERIFY(f_icu_match("\ufee9\ufeed (\ufe8e\ufee0\ufee8\ufebb)",
                     subject_ar, ref(matches), k_UREGEX_OFFSET_CAPTURE));
  VS(f_print_r(matches, true),
    "Array\n"
    "(\n"
    "    [0] => Array\n"
    "        (\n"
    "            [0] => \ufee9\ufeed \ufe8e\ufee0\ufee8\ufebb\n"
    "            [1] => 4\n"
    "        )\n"
    "\n"
    "    [1] => Array\n"
    "        (\n"
    "            [0] => \ufe8e\ufee0\ufee8\ufebb\n"
    "            [1] => 7\n"
    "        )\n"
    "\n"
    ")\n");

  // Test match for 32-bit code points.
  VERIFY(f_icu_match(".*", subject_32, ref(matches)));
  VS(f_print_r(matches, true),
    "Array\n"
    "(\n"
    "    [0] => \U00010905\U00010905\U00010905\U00010905\U00010905\U00010905\n"
    ")\n");

  // Test regex caching functionality.
  VERIFY(f_icu_match("(php)", subject, null, k_UREGEX_CASE_INSENSITIVE));
  VERIFY(!f_icu_match("(php)", subject));

  // Test ICU specific (ie bidi) functionality.
  String pattern_ltr = String("\\p{Bidi_Class=Left_To_Right}", CopyString);
  String pattern_rtl = String("\\p{Bidi_Class=Right_To_Left}", CopyString);
  String pattern_arl = String("\\p{Bidi_Class=Arabic_Letter}", CopyString);

 VERIFY(f_icu_match(pattern_ltr, subject_en));
  VERIFY(!f_icu_match(pattern_rtl, subject_en));

  VERIFY(!f_icu_match(pattern_ltr, subject_he));
  VERIFY(f_icu_match(pattern_rtl, subject_he));
  VERIFY(!f_icu_match(pattern_arl, subject_he));

  VERIFY(!f_icu_match(pattern_ltr, subject_ar));
  VERIFY(!f_icu_match(pattern_rtl, subject_ar));
  VERIFY(f_icu_match(pattern_arl, subject_ar));

  VERIFY(f_icu_match(pattern_ltr, subject_mixed));
  VERIFY(f_icu_match(pattern_rtl, subject_mixed));

  return Count(true);
}

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
