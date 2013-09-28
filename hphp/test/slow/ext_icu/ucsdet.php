<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x, true); }

//////////////////////////////////////////////////////////////////////

// Php doesn't support \u escapes.
function getunicode($x) { return json_decode("\"" . $x . "\""); }

function detect_and_convert_to_utf8($bytes, $utf8) {
  $detector = new EncodingDetector();
  $detector->settext($bytes);
  $match = $detector->detect();
  if (!$match->isvalid()) {
    return false;
  }
  // echo "Got: " . $match->getutf8() . "\n";
  // echo "Want: " . $utf8 . "\n";
  return $match->getutf8() == $utf8;
}

function test_basics() {
  // This is as unmistakably UTF-8 as it gets.
  $utf8_snowman_with_bom = getunicode("\\uFEFF\\u2603");

  $detector = new EncodingDetector();

  $detector->settext($utf8_snowman_with_bom);
  $match = $detector->detect();
  VERIFY($match->isvalid() == true);
  VERIFY($match->getencoding() == "UTF-8");
  VERIFY($match->getconfidence() == 100);
  VERIFY($match->getutf8() == $utf8_snowman_with_bom);
}

function test_cannot_detect() {
  $detector = new EncodingDetector();

  // The detector has no idea what to do with this.
  $detector->settext("\xc7\xe8\xec\xed\xe8\xe9 \xe2\xe5\xf7\xe5\xf0");
  $match = $detector->detect();
  VERIFY($match->isvalid() == false);
}

function test_declared_encoding() {
  // Right now (ICU 4.6), this API doesn't actually do anything, but
  // let's at least verify it doesn't crash.
  $detector = new EncodingDetector();
  $detector->settext("Yo!");
  $detector->setdeclaredencoding("windows-1251");

  $match = $detector->detect();
  VERIFY($match->isvalid() == true);
  VERIFY($match->getutf8() == "Yo!");
}

function test_hello_world() {
  VERIFY(detect_and_convert_to_utf8("Hello, world!", "Hello, world!") == true);
}

function test_windows_1252() {
  VERIFY(detect_and_convert_to_utf8(
    "Toda Europa ley\xf3 Don Quijote como una s\xe1tira.",
    getunicode('Toda Europa ley\u00f3 Don Quijote como una s\u00e1tira.'))
    == true);

  VERIFY(detect_and_convert_to_utf8(
    "Notre P\xe8re, qui \xeates aux cieux",
    getunicode('Notre P\u00e8re, qui \u00eates aux cieux')) == true);

  VERIFY(detect_and_convert_to_utf8(
   "Marta da Silva, als beste Spielerin und beste Torsch\xFCtzin der WM ".
   "2007 sowie bisher f\xFCnf Mal als \x84Weltfu\xDF". "ballerin des ".
   "Jahres\x93 ausgezeichnet, kommt zur Welt.",
   getunicode(
     'Marta da Silva, als beste Spielerin und beste Torsch\u00fctzin der WM '.
     '2007 sowie bisher f\u00FCnf Mal als \u201EWeltfu\u00DFballerin des '.
     'Jahres\u201C ausgezeichnet, kommt zur Welt.')) == true);
}

function test_windows_1250() {
  VERIFY(detect_and_convert_to_utf8(
    "Do Wikipedie m\xf9\x9e" ."e p\xf8isp\xedvat kdokoliv.",
    getunicode('Do Wikipedie m\u016f\u017ee p\u0159isp\u00edvat kdokoliv.'))
    == true);

  VERIFY(detect_and_convert_to_utf8(
    "O\xe8". "e na\x9a, koji jesi na nebesima, sveti se ime Tvoje.",
    getunicode('O\u010de na\u0161, koji jesi na nebesima, sveti se ime Tvoje.'))
    == true);

  VERIFY(detect_and_convert_to_utf8(
    "Prezentacj\xea pierwszego graficznego \x9crodowiska pracy z rodziny ".
    "Windows firmy Microsoft przeprowadzono w listopadzie 1985.",
    getunicode('Prezentacj\u0119 pierwszego graficznego \u015brodowiska pracy z rodziny '.
    'Windows firmy Microsoft przeprowadzono w listopadzie 1985.')) == true);
}

function test_windows_1256() {
  VERIFY(detect_and_convert_to_utf8(
    "\xe1\xc7 \xc3\xca\xdf\xe1\xe3 \xc7\xe1\xda\xd1\xc8\xed\xc9",
    getunicode(
      '\u0644\u0627 \u0623\u062a\u0643\u0644\u0645 \u0627\u0644\u0639\u0631\u0628'.
      '\u064a\u0629')) == true);

  VERIFY(detect_and_convert_to_utf8(
    "\xe6\xed\xdf\xed\xc8\xed\xcf\xed\xc7 \xe5\xed \xe3\xd4\xd1\xe6\xda \xe3".
    "\xe6\xd3\xe6\xda\xc9 \xe3\xca\xda\xcf\xcf\xc9 \xc7\xe1\xe1\xdb\xc7\xca".
    "\xa1 \xe3\xc8\xe4\xed\xc9 \xda\xe1\xec \xc7\xe1\xe6\xed\xc8\xa1 \xd0\xc7".
    "\xca \xe3\xcd\xca\xe6\xec \xcd\xd1\xa1 \xca\xd4\xdb\xe1\xe5\xc7 \xe3\xc4".
    "\xd3\xd3\xc9 \xe6\xed\xdf\xed\xe3\xed\xcf\xed\xc7\xa1 \xc7\xe1\xca".
    "\xed \xe5\xed \xe3\xe4\xd9\xe3\xc9 \xdb\xed\xd1 \xd1\xc8\xcd\xed\xc9.",
  getunicode(
    '\u0648\u064a\u0643\u064a\u0628\u064a\u062f\u064a\u0627 \u0647\u064a \u0645'.
    '\u0634\u0631\u0648\u0639 \u0645\u0648\u0633\u0648\u0639\u0629 \u0645\u062a'.
    '\u0639\u062f\u062f\u0629 \u0627\u0644\u0644\u063a\u0627\u062a\u060c \u0645'.
    '\u0628\u0646\u064a\u0629 \u0639\u0644\u0649 \u0627\u0644\u0648\u064a\u0628'.
    '\u060c \u0630\u0627\u062a \u0645\u062d\u062a\u0648\u0649 \u062d\u0631'.
    '\u060c \u062a\u0634\u063a\u0644\u0647\u0627 \u0645\u0624\u0633\u0633'.
    '\u0629 \u0648\u064a\u0643\u064a\u0645\u064a\u062f\u064a\u0627\u060c \u0627'.
    '\u0644\u062a\u064a \u0647\u064a \u0645\u0646\u0638\u0645\u0629 \u063a'.
    '\u064a\u0631 \u0631\u0628\u062d\u064a\u0629.')) == true);
}

function test_shift_jis() {
  VERIFY(detect_and_convert_to_utf8(
    "\x81w\x82\xc6\x82\xc8\x82\xe8\x82\xcc\x83g\x83g\x83\x8d\x81x\x82\xcd\x81".
    "A\x83X\x83^\x83W\x83I\x83W\x83u\x83\x8a\x90\xa7\x8d\xec\x82\xcc\x93\xfa".
    "\x96{\x82\xcc\x92\xb7\x95\xd2\x83". "A\x83j\x83\x81\x81[\x83V\x83\x87\x83".
    "\x93\x8d\xec\x95i\x81". "B",
  getunicode(
    '\u300e\u3068\u306a\u308a\u306e\u30c8\u30c8\u30ed\u300f\u306f\u3001\u30b9'.
    '\u30bf\u30b8\u30aa\u30b8\u30d6\u30ea\u5236\u4f5c\u306e\u65e5\u672c\u306e'.
    '\u9577\u7de8\u30a2\u30cb\u30e1\u30fc\u30b7\u30e7\u30f3\u4f5c\u54c1\u3002'))
         == true);

  VERIFY(detect_and_convert_to_utf8(
    "\x83". "E\x83". "B\x83L\x83y\x83". "f\x83". "B\x83". "A (Wikipedia) \x82\xcd".
    "\x83". "E\x83". "B\x83L\x83\x81\x83". "f\x83". "B\x83". "A\x8d\xe0\x92".
    "c\x82\xaa\x89^\x89". "c\x82\xb7\x82\xe9\x83I\x83\x93\x83\x89\x83". "C\x83".
    "\x93\x95S\x89\xc8\x8e\x96\x93T\x81". "B",
    getunicode(
    '\u30a6\u30a3\u30ad\u30da\u30c7\u30a3\u30a2 (Wikipedia) \u306f\u30a6'.
    '\u30a3\u30ad\u30e1\u30c7\u30a3\u30a2\u8ca1\u56e3\u304c\u904b\u55b6\u3059'.
    '\u308b\u30aa\u30f3\u30e9\u30a4\u30f3\u767e\u79d1\u4e8b\u5178\u3002'))
         == true);

  // Too short; detector thinks it's most likely Windows-1252.
  //
  // VERIFY(detect_and_convert_to_utf8(
  //   "\x90\xa2\x8a" "E\x90l\x8c\xa0\x90\xe9\x8c\xbe",
  //   "\u4e16\u754c\u4eba\u6a29\u5ba3\u8a00") == true);
}

function test_euc_jp() {
  VERIFY(detect_and_convert_to_utf8(
    "\xb2\xbf\xbf\xcd\xa4\xe2\xa1\xa2\xa4\xdb\xa4\xb7\xa4\xa4\xa4\xde\xa4\xde".
    "\xa4\xcb\xc2\xe1\xca\xe1\xa1\xa2\xb9\xb4\xb6\xd8\xa1\xa2\xcb\xf4\xa4\xcf".
    "\xc4\xc9\xca\xfc\xa4\xb5\xa4\xec\xa4\xeb\xa4\xb3\xa4\xc8\xa4\xcf\xa4\xca".
    "\xa4\xa4\xa1\xa3",
    getunicode(
    '\u4f55\u4eba\u3082\u3001\u307b\u3057\u3044\u307e\u307e\u306b\u902e\u6355'.
    '\u3001\u62d8\u7981\u3001\u53c8\u306f\u8ffd\u653e\u3055\u308c\u308b\u3053'.
    '\u3068\u306f\u306a\u3044\u3002')) == true);
  VERIFY(detect_and_convert_to_utf8(
    "1920\xc7\xaf \xa5\xa6\xa5\xa3\xa5\xf3\xa5\xd6\xa5\xeb\xa5\xc9\xa5\xf3\xc1".
    "\xaa\xbc\xea\xb8\xa2\xa1\xcaThe Championships, Wimbledon 1920\xa1\xcb\xa4".
    "\xcb\xb4\xd8\xa4\xb9\xa4\xeb\xb5\xad\xbb\xf6\xa1\xa3",
    getunicode(
    '1920\u5e74 \u30a6\u30a3\u30f3\u30d6\u30eb\u30c9\u30f3\u9078\u624b\u6a29'.
    '\uff08The Championships, Wimbledon 1920\uff09\u306b\u95a2\u3059\u308b'.
    '\u8a18\u4e8b\u3002')) == true);
}

function test_iso_2022_jp() {
  VERIFY(detect_and_convert_to_utf8(
    "\x1b\$B%-%M%F%#%C%/%3%M%/%7%g%s!J\x1b(Bkinetic connection\x1b\$B!K\$O\x1b(B1".
    "986\x1b\$BG/\$K%=%K!<\$,\x1b(BMSX2\x1b\$B\$GH/Gd\$7\$?%Q%:%k%2!<%`!#\x1b(B",
    getunicode(
    '\u30ad\u30cd\u30c6\u30a3\u30c3\u30af\u30b3\u30cd\u30af\u30b7\u30e7\u30f3'.
    '\uff08kinetic connection\uff09\u306f1986\u5e74\u306b\u30bd\u30cb\u30fc'.
    '\u304cMSX2\u3067\u767a\u58f2\u3057\u305f\u30d1\u30ba\u30eb\u30b2\u30fc'.
    '\u30e0\u3002')) == true);
  VERIFY(detect_and_convert_to_utf8(
    "\x1b\$B5f6K\x1b(B!!\x1b\$BJQBV2>LL\x1b(B",
    getunicode("\u7a76\u6975!!\u5909\u614b\u4eee\u9762")) == true);
}

function test_gb2312() {
  VERIFY(detect_and_convert_to_utf8(
    "\xca\xc7\xd2\xbb\xb8\xf6\xd3\xef\xd1\xd4\xa1\xa2\xc4\xda\xc8\xdd\xbf\xaa".
    "\xb7\xc5\xb5\xc4\xcd\xf8\xc2\xe7\xb0\xd9\xbf\xc6\xc8\xab\xca\xe9\xbc\xc6".
    "\xbb\xae",
    getunicode(
    '\u662f\u4e00\u4e2a\u8bed\u8a00\u3001\u5185\u5bb9\u5f00\u653e\u7684\u7f51'.
    '\u7edc\u767e\u79d1\u5168\u4e66\u8ba1\u5212')) == true);
  VERIFY(detect_and_convert_to_utf8(
    "\xa1\xb6\xd2\xbb\xc1\xa3\xd6\xd3\xd5\xe6\xc8\xcb\xcb\xd5\xa1\xb7\xa3\xa8".
    "\xd3\xa2\xce\xc4\xc3\xfb\xa3\xbaSo Real Time Cooking\xa3\xa9",
    getunicode(
    '\u300a\u4e00\u7c92\u949f\u771f\u4eba\u82cf\u300b\uff08\u82f1\u6587\u540d'.
    '\uff1aSo Real Time Cooking\uff09')) == true);

  // Too short; detector thinks it's most likely Shift-JIS.
  // VERIFY(detect_and_convert_to_utf8(
  //   "\xce\xe2\xb8\xe7\xbf\xdf",
  //   "\u5434\u54e5\u7a9f") == true);
}

function test_big5() {
  VERIFY(detect_and_convert_to_utf8(
    "1\xa1]\xa4@\xa1^\xacO0\xbbP2\xa4\xa7\xb6\xa1\xaa\xba\xa6\xdb\xb5M\xbc\xc6".
    "\xa1". "A\xacO\xb3\xcc\xa4p\xaa\xba\xa5\xbf\xa9_\xbc\xc6\xa1". "C",
    getunicode(
    '1\uff08\u4e00\uff09\u662f0\u82072\u4e4b\u9593\u7684\u81ea\u7136\u6578'.
    '\uff0c\u662f\u6700\u5c0f\u7684\u6b63\u5947\u6578\u3002')) == true);
  VERIFY(detect_and_convert_to_utf8(
    "\xbeG\xa5\xf2\xaf\xf4\xa1]\xad^\xa4\xe5\xa6W \xa1G Cheng Chung Yin\xa1^".
    "\xa1". "A\xacO\xa4@\xa6W\xa5x\xc6W\xa4k\xbat\xad\xfb",
    getunicode(
    '\u912d\u4ef2\u8335\uff08\u82f1\u6587\u540d \uff1a Cheng Chung Yin\uff09'.
    '\uff0c\u662f\u4e00\u540d\u53f0\u7063\u5973\u6f14\u54e1')) == true);

  // Too short; detector thinks it's most likely Shift-JIS.
  // VERIFY(detect_and_convert_to_utf8(
  //   "\xa7" "d\xad\xf4\xb8]",
  //   "\u5433\u54e5\u7a9f") == true);
}

function test_koi8r() {
  VERIFY(detect_and_convert_to_utf8(
    "\xeb\xd7\xc5\xc2\xc5\xcb \xd0\xc5\xd2\xd7\xc1\xd1 \xd0\xcf \xd0\xcc\xcf".
    "\xdd\xc1\xc4\xc9 \xc9 \xd7\xd4\xcf\xd2\xc1\xd1 \xd0\xcf \xce\xc1\xd3\xc5".
    "\xcc\xc5\xce\xc9\xc0 \xd0\xd2\xcf\xd7\xc9\xce\xc3\xc9\xd1 \xeb\xc1\xce".
    "\xc1\xc4\xd9.",
    getunicode(
    '\u041a\u0432\u0435\u0431\u0435\u043a \u043f\u0435\u0440\u0432\u0430'.
    '\u044f \u043f\u043e \u043f\u043b\u043e\u0449\u0430\u0434\u0438 \u0438 '.
    '\u0432\u0442\u043e\u0440\u0430\u044f \u043f\u043e \u043d\u0430\u0441'.
    '\u0435\u043b\u0435\u043d\u0438\u044e \u043f\u0440\u043e\u0432\u0438'.
    '\u043d\u0446\u0438\u044f \u041a\u0430\u043d\u0430\u0434\u044b.')) == true);
  VERIFY(detect_and_convert_to_utf8(
    "\xe2\xdf\xd2\xc4\xc5\xce\xc9 \xc5 \xd3\xc5\xcc\xcf \xd7 \xf3\xc5\xd7\xc5".
    "\xd2\xce\xc1 \xe2\xdf\xcc\xc7\xc1\xd2\xc9\xd1",
    getunicode(
    '\u0411\u044a\u0440\u0434\u0435\u043d\u0438 \u0435 \u0441\u0435\u043b'.
    '\u043e \u0432 \u0421\u0435\u0432\u0435\u0440\u043d\u0430 \u0411\u044a'.
    '\u043b\u0433\u0430\u0440\u0438\u044f')) == true);
  VERIFY(detect_and_convert_to_utf8(
    "\xfe\xc5\xd2\xd7\xc5\xce\xc1 \xd0\xd2\xc5\xd7\xdf\xda\xc8\xcf\xc4\xce".
    "\xc1",
    getunicode(
    '\u0427\u0435\u0440\u0432\u0435\u043d\u0430 \u043f\u0440\u0435\u0432'.
    '\u044a\u0437\u0445\u043e\u0434\u043d\u0430')) == true);
}

function test_windows_1251() {
  VERIFY(detect_and_convert_to_utf8(
    "\xce\xf7\xe5 \xed\xe0\xf8, \xea\xee\xbc \xf1\xe8 \xed\xe0 \xed\xe5\xe1".
    "\xe5\xf1\xe0\xf2\xe0",
    getunicode(
    '\u041e\u0447\u0435 \u043d\u0430\u0448, \u043a\u043e\u0458 \u0441\u0438 '.
    '\u043d\u0430 \u043d\u0435\u0431\u0435\u0441\u0430\u0442\u0430')) == true);
  VERIFY(detect_and_convert_to_utf8(
    "\xd1 \xe7\xee\xee\xeb\xee\xe3\xe8\xf7\xe5\xf1\xea\xee\xe9 \xf2\xee\xf7\xea".
    "\xe8 \xe7\xf0\xe5\xed\xe8\xff, \xe4\xee\xec\xe0\xf8\xed\xff\xff \xea\xee".
    "\xf8\xea\xe0 \x97 \xec\xeb\xe5\xea\xee\xef\xe8\xf2\xe0\xfe\xf9\xe5\xe5".
    " \xf1\xe5\xec\xe5\xe9\xf1\xf2\xe2\xe0 \xea\xee\xf8\xe0\xf7\xfc\xe8\xf5 ".
    "\xee\xf2\xf0\xff\xe4\xe0 \xf5\xe8\xf9\xed\xfb\xf5.",
    getunicode(
    '\u0421 \u0437\u043e\u043e\u043b\u043e\u0433\u0438\u0447\u0435\u0441\u043a'.
    '\u043e\u0439 \u0442\u043e\u0447\u043a\u0438 \u0437\u0440\u0435\u043d\u0438'.
    '\u044f, \u0434\u043e\u043c\u0430\u0448\u043d\u044f\u044f \u043a\u043e'.
    '\u0448\u043a\u0430 \u2014 \u043c\u043b\u0435\u043a\u043e\u043f\u0438\u0442'.
    '\u0430\u044e\u0449\u0435\u0435 \u0441\u0435\u043c\u0435\u0439\u0441\u0442'.
    '\u0432\u0430 \u043a\u043e\u0448\u0430\u0447\u044c\u0438\u0445 \u043e\u0442'.
    '\u0440\u044f\u0434\u0430 \u0445\u0438\u0449\u043d\u044b\u0445.')) == true);
}

function test_utf8() {
  VERIFY(detect_and_convert_to_utf8(
    getunicode(
    "\u10e8\u10d8\u10dc\u10d0\u10e3\u10e0\u10d8 \u10d9\u10d0\u10e2\u10d0"),
    getunicode(
    '\u10e8\u10d8\u10dc\u10d0\u10e3\u10e0\u10d8 \u10d9\u10d0\u10e2\u10d0'))
         == true);
  VERIFY(detect_and_convert_to_utf8(
    getunicode('\u0e2b\u0e19\u0e49\u0e32\u0e2b\u0e25\u0e31\u0e01'),
    getunicode('\u0e2b\u0e19\u0e49\u0e32\u0e2b\u0e25\u0e31\u0e01')) == true);
}

function test_utf16() {
  // The detector only handles UTF-16 if there's a BOM at the front.
  $utf16 =
    "\xff\xfeH\x00". "e\x00l\x00l\x00o\x00,\x00 \x00w\x00o\x00r\x00l\x00".
    "d\x00!\x00";
  VERIFY(detect_and_convert_to_utf8(
    $utf16,
    getunicode("\ufeffHello, world!")) == true);
}

test_basics();
test_cannot_detect();
test_declared_encoding();
test_hello_world();
test_windows_1252();
test_windows_1250();
test_windows_1256();
test_shift_jis();
test_euc_jp();
test_iso_2022_jp();
test_gb2312();
test_big5();
test_koi8r();
test_windows_1251();
test_utf8();
test_utf16();
