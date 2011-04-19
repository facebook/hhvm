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

#include <test/test_ext_icu_uspoof.h>
#include <runtime/ext/ext_icu_uspoof.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtIcu_uspoof::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_SpoofChecker_issuspicious);
  RUN_TEST(test_SpoofChecker_areconfusable);
  RUN_TEST(test_SpoofChecker_issuesfound);
  RUN_TEST(test_SpoofChecker_setchecks);
  RUN_TEST(test_SpoofChecker_setallowedlocales);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIcu_uspoof::test_SpoofChecker_issuspicious() {
  p_SpoofChecker checker(NEWOBJ(c_SpoofChecker)());
  VS(checker->t_issuspicious("facebook"), false);

  // facebook with Cyrillic spoof characters
  VS(checker->t_issuspicious("f\u0430\u0441\u0435b\u043e\u043ek"), true);

  // "Russia" in Cyrillic with Latin spoof characters
  VS(checker->t_issuspicious("Pocc\u0438\u044f"), true);

  // paypal with Cyrillic spoof characters
  VS(checker->t_issuspicious("http://www.payp\u0430l.com"), true);

  // certain all-uppercase Latin sequences can be spoof of Greek
  VS(checker->t_issuspicious("NAPKIN PEZ"), true);
  VS(checker->t_issuspicious("napkin pez"), false);

  // English with Japanese characters
  VS(checker->t_issuspicious("True fact: \u5fcd\u8005 are mammals"), false);

  // Japanese name with mixed kanji and hiragana
  VS(checker->t_issuspicious("\u6a4b\u672c\u611b\u307f"), false);

  try {
    checker->t_issuspicious("this is not UTF-8: \x87\xFB\xCA\x94\xDB");
  } catch (Exception& e) {
    return Count(true);
  }

  return Count(false);
}

bool TestExtIcu_uspoof::test_SpoofChecker_areconfusable() {
  p_SpoofChecker checker(NEWOBJ(c_SpoofChecker)());
  VS(checker->t_areconfusable("hello, world", "goodbye, world"), false);
  VS(checker->t_areconfusable("hello, world", "hello, world"), true);
  VS(checker->t_areconfusable("hello, world", "he11o, wor1d"), true);
  VS(checker->t_areconfusable("hell\u00f8", "hello\u0337"), true);

  VS(checker->t_areconfusable("facebook", "f\u0430\u0441\u0435b\u043e\u043ek"),
     true);

  VS(checker->t_areconfusable("facebook", "\U0001d41faceboo\u1d0b"), true);

  // TODO: ICU bug 8341: \u017f should be treated as a spoof of "f".  Once
  // that bug is fixed, enable this test.
  //
  // VS(checker->t_areconfusable("facebook", "\u017facebook"), true);

  VS(checker->t_areconfusable("paypal", "payp\u0430l"), true);
  VS(checker->t_areconfusable(
       "NAPKIN PEZ",
       "\u039d\u0391\u03a1\u039a\u0399\u039d \u03a1\u0395\u0396"),
     true);

  VS(checker->t_areconfusable(
       "facebook",
       "ufiek-a\u048ba\u049d \u049da\u048b\u00f0a\u048b\u01e5a\u048b-\u049dota-"
       "\u00f0o\u00f0ol"),
     false);

  try {
    checker->t_areconfusable(
      "this is not UTF-8: \x87\xFB\xCA\x94\xDB",
      "so there.");
  } catch (Exception& e) {
    return Count(true);
  }

  return Count(false);
}

bool TestExtIcu_uspoof::test_SpoofChecker_issuesfound() {
  p_SpoofChecker checker(NEWOBJ(c_SpoofChecker)());
  Variant ret;

  VS(checker->t_issuspicious("NAPKIN PEZ", ref(ret)), true);
  VS(ret.getInt64(), q_SpoofChecker_WHOLE_SCRIPT_CONFUSABLE);

  VS(checker->t_issuspicious("f\u0430\u0441\u0435b\u043e\u043ek", ref(ret)),
     true);
  VS(ret.getInt64(), q_SpoofChecker_MIXED_SCRIPT_CONFUSABLE);

  VS(checker->t_areconfusable("hello, world", "he11o, wor1d", ref(ret)), true);
  VS(ret.getInt64(), q_SpoofChecker_SINGLE_SCRIPT_CONFUSABLE);

  return Count(true);
}

bool TestExtIcu_uspoof::test_SpoofChecker_setchecks() {
  {
    p_SpoofChecker checker(NEWOBJ(c_SpoofChecker)());

    // The checker should start in any-case mode.
    VS(checker->t_areconfusable("HELLO", "H\u0415LLO"), true);
    VS(checker->t_areconfusable("hello", "h\u0435llo"), true);

    // Go to lower-case only mode (assumes all strings have been
    // case-folded).
    checker->t_setchecks(
      q_SpoofChecker_MIXED_SCRIPT_CONFUSABLE |
      q_SpoofChecker_WHOLE_SCRIPT_CONFUSABLE |
      q_SpoofChecker_SINGLE_SCRIPT_CONFUSABLE
    );
    VS(checker->t_areconfusable("HELLO", "H\u0415LLO"), false);
    VS(checker->t_areconfusable("hello", "h\u0435llo"), true);
  }

  {
    p_SpoofChecker checker(NEWOBJ(c_SpoofChecker)());
    VS(checker->t_issuspicious("True fact: \u5fcd\u8005 are mammals"), false);

    // Only allow characters of a single script.
    checker->t_setchecks(q_SpoofChecker_SINGLE_SCRIPT);
    VS(checker->t_issuspicious("True fact: \u5fcd\u8005 are mammals"), true);
  }

  try {
    p_SpoofChecker checker(NEWOBJ(c_SpoofChecker)());
    checker->t_setchecks(0xDEADBEEF);
  } catch (Exception& e) {
    return Count(true);
  }

  return Count(false);
}

bool TestExtIcu_uspoof::test_SpoofChecker_setallowedlocales() {
  p_SpoofChecker checker(NEWOBJ(c_SpoofChecker)());

  const char* common = "Rogers";
  const char* japanese_kanji_hiragana = "\u6a4b\u672c\u611b\u307f";
  const char* korean = "\ud55c\uad6d\ub9d0";
  const char* arabic = "\u0645\u0631\u062d\u0628\u064b\u0627";
  const char* russian_cyrillic =
    "\u0417\u0438\u0301\u043c\u043d\u0438\u0439 "
    "\u0432\u0435\u0301\u0447\u0435\u0440";
  const char* snowman = "\u2603";

  checker->t_setallowedlocales("en_US");
  VS(checker->t_issuspicious(common), false);
  VS(checker->t_issuspicious(japanese_kanji_hiragana), true);
  VS(checker->t_issuspicious(russian_cyrillic), true);
  VS(checker->t_issuspicious(arabic), true);
  VS(checker->t_issuspicious(korean), true);
  VS(checker->t_issuspicious(snowman), false);

  checker->t_setallowedlocales("en_US, ja_JP");
  VS(checker->t_issuspicious(common), false);
  VS(checker->t_issuspicious(japanese_kanji_hiragana), false);
  VS(checker->t_issuspicious(russian_cyrillic), true);
  VS(checker->t_issuspicious(arabic), true);
  VS(checker->t_issuspicious(korean), true);
  VS(checker->t_issuspicious(snowman), false);

  checker->t_setallowedlocales("en_US, ko_KR");
  VS(checker->t_issuspicious(common), false);
  VS(checker->t_issuspicious(japanese_kanji_hiragana), true);
  VS(checker->t_issuspicious(russian_cyrillic), true);
  VS(checker->t_issuspicious(arabic), true);
  VS(checker->t_issuspicious(korean), false);
  VS(checker->t_issuspicious(snowman), false);

  checker->t_setallowedlocales("en_US, ar_AR");
  VS(checker->t_issuspicious(common), false);
  VS(checker->t_issuspicious(japanese_kanji_hiragana), true);
  VS(checker->t_issuspicious(russian_cyrillic), true);
  VS(checker->t_issuspicious(arabic), false);
  VS(checker->t_issuspicious(korean), true);
  VS(checker->t_issuspicious(snowman), false);

  checker->t_setallowedlocales("en_US, ru_RU");
  VS(checker->t_issuspicious(common), false);
  VS(checker->t_issuspicious(japanese_kanji_hiragana), true);
  VS(checker->t_issuspicious(russian_cyrillic), false);
  VS(checker->t_issuspicious(arabic), true);
  VS(checker->t_issuspicious(korean), true);
  VS(checker->t_issuspicious(snowman), false);

  return Count(true);
}
