<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) :mixed{ VS($x, true); }

//////////////////////////////////////////////////////////////////////

// Php doesn't support \u escapes.
function u($x) :mixed{ return json_decode("\"" . $x . "\""); }

function test_SpoofChecker_issuspicious(): void {
  $issues = null;
  $checker = new SpoofChecker();
  VS($checker->isSuspicious("facebook", inout $issues), false);

  // facebook with Cyrillic spoof characters
  VS($checker->isSuspicious(u('f\u0430\u0441\u0435b\u043e\u043ek'), inout $issues), true);

  // "Russia" in Cyrillic with Latin spoof characters
  VS($checker->isSuspicious(u('Pocc\u0438\u044f'), inout $issues), true);

  // paypal with Cyrillic spoof characters
  VS($checker->isSuspicious(u('http://www.payp\u0430l.com'), inout $issues), true);

  // certain all-uppercase Latin sequences can be spoof of Greek
  VS($checker->isSuspicious('NAPKIN PEZ', inout $issues), true);
  VS($checker->isSuspicious('napkin pez', inout $issues), false);

  // English with Japanese characters
  VS($checker->isSuspicious(u('True fact: \u5fcd\u8005 are mammals'), inout $issues), false);

  // Japanese name with mixed kanji and hiragana
  VS($checker->isSuspicious(u('\u6a4b\u672c\u611b\u307f'), inout $issues), false);

  // try {
  //   $checker->issuspicious("this is not UTF-8: \x87\xFB\xCA\x94\xDB");
  // } catch (Exception $e) {
  //   VS(true, true);
  // }
}

function test_SpoofChecker_areconfusable(): void {
  $issues = null;
  $checker = new SpoofChecker();
  VS($checker->areConfusable("hello, world", "goodbye, world", inout $issues), false);
  VS($checker->areConfusable("hello, world", "hello, world", inout $issues), true);
  VS($checker->areConfusable("hello, world", "he11o, wor1d", inout $issues), true);
  VS($checker->areConfusable(u('hell\u00f8'), u('hello\u0337'), inout $issues), true);

  VS($checker->areConfusable("facebook",
      u('f\u0430\u0441\u0435b\u043e\u043ek'),
      inout $issues),
     true);

  VS($checker->areConfusable("facebook", "\xf0\x9d\x90\x9faceboo".u('\u1d0b'), inout $issues),
     true);

  VS($checker->areConfusable("facebook", u('\u017facebook'), inout $issues), true);

  VS($checker->areConfusable("paypal", u('payp\u0430l'), inout $issues), true);
  VS($checker->areConfusable(
       "NAPKIN PEZ",
       u('\u039d\u0391\u03a1\u039a\u0399\u039d \u03a1\u0395\u0396'),
       inout $issues),
     true);

  VS($checker->areConfusable(
       "facebook",
       u('ufiek-a\u048ba\u049d \u049da\u048b\u00f0a\u048b\u01e5a\u048b-\u049dota-'.
         '\u00f0o\u00f0ol'),
       inout $issues),
     false);

  // try {
  //   $checker->areconfusable(
  //     "this is not UTF-8: \x87\xFB\xCA\x94\xDB",
  //     "so there.");
  // } catch (Exception $e) {
  //   VS(true, true);
  // }
}

function test_SpoofChecker_issuesfound(): void {
  $ret = null;
  $checker = new SpoofChecker();

  VS($checker->isSuspicious("NAPKIN PEZ", inout $ret), true);
  VS($ret, SpoofChecker::WHOLE_SCRIPT_CONFUSABLE);

  VS($checker->isSuspicious(u('f\u0430\u0441\u0435b\u043e\u043ek'), inout $ret),
     true);
  VS($ret, SpoofChecker::MIXED_SCRIPT_CONFUSABLE);

  VS($checker->areConfusable("hello, world", "he11o, wor1d", inout $ret), true);
  VS($ret, SpoofChecker::SINGLE_SCRIPT_CONFUSABLE);
}

function test_SpoofChecker_setchecks(): void {
  $issues = null;
  $checker = new SpoofChecker();

  // The checker should start in any-case mode.
  VS($checker->areConfusable("HELLO", u('H\u0415LLO'), inout $issues), true);
  VS($checker->areConfusable("hello", u('h\u0435llo'), inout $issues), true);

  // Go to lower-case only mode (assumes all strings have been
  // case-folded).
  $checker->setChecks(
    SpoofChecker::MIXED_SCRIPT_CONFUSABLE |
    SpoofChecker::WHOLE_SCRIPT_CONFUSABLE |
    SpoofChecker::SINGLE_SCRIPT_CONFUSABLE
  );
  VS($checker->areConfusable("HELLO", u('H\u0415LLO'), inout $issues), false);
  VS($checker->areConfusable("hello", u('h\u0435llo'), inout $issues), true);

  $checker = new SpoofChecker();
  VS($checker->isSuspicious(u('True fact: \u5fcd\u8005 are mammals'), inout $issues), false);

  // try {
  //   $checker = new SpoofChecker();
  //   $checker->setchecks(0xDEADBEEF);
  // } catch (Exception $e) {
  //   VS(true, true);
  // }
}

function test_SpoofChecker_setAllowedLocales(): void {
  $issues = null;
  $checker = new SpoofChecker();

  $common = "Rogers";
  $japanese_kanji_hiragana = u('\u6a4b\u672c\u611b\u307f');
  $korean = u('\ud55c\uad6d\ub9d0');
  $arabic = u('\u0645\u0631\u062d\u0628\u064b\u0627');
  $russian_cyrillic =
    u('\u0417\u0438\u0301\u043c\u043d\u0438\u0439 '.
      '\u0432\u0435\u0301\u0447\u0435\u0440');
  $snowman = u('\u2603');

  $checker->setAllowedLocales("en_US");
  VS($checker->isSuspicious($common, inout $issues), false);
  VS($checker->isSuspicious($japanese_kanji_hiragana, inout $issues), true);
  VS($checker->isSuspicious($russian_cyrillic, inout $issues), true);
  VS($checker->isSuspicious($arabic, inout $issues), true);
  VS($checker->isSuspicious($korean, inout $issues), true);
  VS($checker->isSuspicious($snowman, inout $issues), false);

  $checker->setAllowedLocales("en_US, ja_JP");
  VS($checker->isSuspicious($common, inout $issues), false);
  VS($checker->isSuspicious($japanese_kanji_hiragana, inout $issues), false);
  VS($checker->isSuspicious($russian_cyrillic, inout $issues), true);
  VS($checker->isSuspicious($arabic, inout $issues), true);
  VS($checker->isSuspicious($korean, inout $issues), true);
  VS($checker->isSuspicious($snowman, inout $issues), false);

  $checker->setAllowedLocales("en_US, ko_KR");
  VS($checker->isSuspicious($common, inout $issues), false);
  VS($checker->isSuspicious($japanese_kanji_hiragana, inout $issues), true);
  VS($checker->isSuspicious($russian_cyrillic, inout $issues), true);
  VS($checker->isSuspicious($arabic, inout $issues), true);
  VS($checker->isSuspicious($korean, inout $issues), false);
  VS($checker->isSuspicious($snowman, inout $issues), false);

  $checker->setAllowedLocales("en_US, ar_AR");
  VS($checker->isSuspicious($common, inout $issues), false);
  VS($checker->isSuspicious($japanese_kanji_hiragana, inout $issues), true);
  VS($checker->isSuspicious($russian_cyrillic, inout $issues), true);
  VS($checker->isSuspicious($arabic, inout $issues), false);
  VS($checker->isSuspicious($korean, inout $issues), true);
  VS($checker->isSuspicious($snowman, inout $issues), false);

  $checker->setAllowedLocales("en_US, ru_RU");
  VS($checker->isSuspicious($common, inout $issues), false);
  VS($checker->isSuspicious($japanese_kanji_hiragana, inout $issues), true);
  VS($checker->isSuspicious($russian_cyrillic, inout $issues), false);
  VS($checker->isSuspicious($arabic, inout $issues), true);
  VS($checker->isSuspicious($korean, inout $issues), true);
  VS($checker->isSuspicious($snowman, inout $issues), false);
}

<<__EntryPoint>>
function main_uspoof() :mixed{
test_SpoofChecker_issuspicious();
test_SpoofChecker_areconfusable();
test_SpoofChecker_issuesfound();
test_SpoofChecker_setchecks();
test_SpoofChecker_setAllowedLocales();
}
