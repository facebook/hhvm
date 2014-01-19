<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x, true); }

//////////////////////////////////////////////////////////////////////

// Php doesn't support \u escapes.
function u($x) { return json_decode("\"" . $x . "\""); }

function test_SpoofChecker_issuspicious() {
  $checker = new SpoofChecker();
  VS($checker->issuspicious("facebook"), false);

  // facebook with Cyrillic spoof characters
  VS($checker->issuspicious(u('f\u0430\u0441\u0435b\u043e\u043ek')), true);

  // "Russia" in Cyrillic with Latin spoof characters
  VS($checker->issuspicious(u('Pocc\u0438\u044f')), true);

  // paypal with Cyrillic spoof characters
  VS($checker->issuspicious(u('http://www.payp\u0430l.com')), true);

  // certain all-uppercase Latin sequences can be spoof of Greek
  VS($checker->issuspicious('NAPKIN PEZ'), true);
  VS($checker->issuspicious('napkin pez'), false);

  // English with Japanese characters
  VS($checker->issuspicious(u('True fact: \u5fcd\u8005 are mammals')), false);

  // Japanese name with mixed kanji and hiragana
  VS($checker->issuspicious(u('\u6a4b\u672c\u611b\u307f')), false);

  // try {
  //   $checker->issuspicious("this is not UTF-8: \x87\xFB\xCA\x94\xDB");
  // } catch (Exception $e) {
  //   VS(true, true);
  // }
}

function test_SpoofChecker_areconfusable() {
  $checker = new SpoofChecker();
  VS($checker->areconfusable("hello, world", "goodbye, world"), false);
  VS($checker->areconfusable("hello, world", "hello, world"), true);
  VS($checker->areconfusable("hello, world", "he11o, wor1d"), true);
  VS($checker->areconfusable(u('hell\u00f8'), u('hello\u0337')), true);

  VS($checker->areconfusable("facebook",
      u('f\u0430\u0441\u0435b\u043e\u043ek')),
     true);

  VS($checker->areconfusable("facebook", "\xf0\x9d\x90\x9faceboo".u('\u1d0b')),
    true);

  VS($checker->areconfusable("facebook", u('\u017facebook')), true);

  VS($checker->areconfusable("paypal", u('payp\u0430l')), true);
  VS($checker->areconfusable(
       "NAPKIN PEZ",
       u('\u039d\u0391\u03a1\u039a\u0399\u039d \u03a1\u0395\u0396')),
     true);

  VS($checker->areconfusable(
       "facebook",
       u('ufiek-a\u048ba\u049d \u049da\u048b\u00f0a\u048b\u01e5a\u048b-\u049dota-'.
         '\u00f0o\u00f0ol')),
     false);

  // try {
  //   $checker->areconfusable(
  //     "this is not UTF-8: \x87\xFB\xCA\x94\xDB",
  //     "so there.");
  // } catch (Exception $e) {
  //   VS(true, true);
  // }
}

function test_SpoofChecker_issuesfound() {
  $checker = new SpoofChecker();

  VS($checker->issuspicious("NAPKIN PEZ", $ret), true);
  VS($ret, Spoofchecker::WHOLE_SCRIPT_CONFUSABLE);

  VS($checker->issuspicious(u('f\u0430\u0441\u0435b\u043e\u043ek'), $ret),
     true);
  VS($ret, SpoofChecker::MIXED_SCRIPT_CONFUSABLE);

  VS($checker->areconfusable("hello, world", "he11o, wor1d", $ret), true);
  VS($ret, SpoofChecker::SINGLE_SCRIPT_CONFUSABLE);

  return Count(true);
}

function test_SpoofChecker_setchecks() {
  $checker = new SpoofChecker();

  // The checker should start in any-case mode.
  VS($checker->areconfusable("HELLO", u('H\u0415LLO')), true);
  VS($checker->areconfusable("hello", u('h\u0435llo')), true);

  // Go to lower-case only mode (assumes all strings have been
  // case-folded).
  $checker->setchecks(
    SpoofChecker::MIXED_SCRIPT_CONFUSABLE |
    SpoofChecker::WHOLE_SCRIPT_CONFUSABLE |
    SpoofChecker::SINGLE_SCRIPT_CONFUSABLE
  );
  VS($checker->areconfusable("HELLO", u('H\u0415LLO')), false);
  VS($checker->areconfusable("hello", u('h\u0435llo')), true);

  $checker = new SpoofChecker();
  VS($checker->issuspicious(u('True fact: \u5fcd\u8005 are mammals')), false);

  // Only allow characters of a single script.
  $checker->setchecks(SpoofChecker::SINGLE_SCRIPT);
  VS($checker->issuspicious(u('True fact: \u5fcd\u8005 are mammals')), true);

  // try {
  //   $checker = new SpoofChecker();
  //   $checker->setchecks(0xDEADBEEF);
  // } catch (Exception $e) {
  //   VS(true, true);
  // }
}

function test_SpoofChecker_setallowedlocales() {
  $checker = new SpoofChecker();

  $common = "Rogers";
  $japanese_kanji_hiragana = u('\u6a4b\u672c\u611b\u307f');
  $korean = u('\ud55c\uad6d\ub9d0');
  $arabic = u('\u0645\u0631\u062d\u0628\u064b\u0627');
  $russian_cyrillic =
    u('\u0417\u0438\u0301\u043c\u043d\u0438\u0439 '.
      '\u0432\u0435\u0301\u0447\u0435\u0440');
  $snowman = u('\u2603');

  $checker->setallowedlocales("en_US");
  VS($checker->issuspicious($common), false);
  VS($checker->issuspicious($japanese_kanji_hiragana), true);
  VS($checker->issuspicious($russian_cyrillic), true);
  VS($checker->issuspicious($arabic), true);
  VS($checker->issuspicious($korean), true);
  VS($checker->issuspicious($snowman), false);

  $checker->setallowedlocales("en_US, ja_JP");
  VS($checker->issuspicious($common), false);
  VS($checker->issuspicious($japanese_kanji_hiragana), false);
  VS($checker->issuspicious($russian_cyrillic), true);
  VS($checker->issuspicious($arabic), true);
  VS($checker->issuspicious($korean), true);
  VS($checker->issuspicious($snowman), false);

  $checker->setallowedlocales("en_US, ko_KR");
  VS($checker->issuspicious($common), false);
  VS($checker->issuspicious($japanese_kanji_hiragana), true);
  VS($checker->issuspicious($russian_cyrillic), true);
  VS($checker->issuspicious($arabic), true);
  VS($checker->issuspicious($korean), false);
  VS($checker->issuspicious($snowman), false);

  $checker->setallowedlocales("en_US, ar_AR");
  VS($checker->issuspicious($common), false);
  VS($checker->issuspicious($japanese_kanji_hiragana), true);
  VS($checker->issuspicious($russian_cyrillic), true);
  VS($checker->issuspicious($arabic), false);
  VS($checker->issuspicious($korean), true);
  VS($checker->issuspicious($snowman), false);

  $checker->setallowedlocales("en_US, ru_RU");
  VS($checker->issuspicious($common), false);
  VS($checker->issuspicious($japanese_kanji_hiragana), true);
  VS($checker->issuspicious($russian_cyrillic), false);
  VS($checker->issuspicious($arabic), true);
  VS($checker->issuspicious($korean), true);
  VS($checker->issuspicious($snowman), false);
}

test_SpoofChecker_issuspicious();
test_SpoofChecker_areconfusable();
test_SpoofChecker_issuesfound();
test_SpoofChecker_setchecks();
test_SpoofChecker_setallowedlocales();
