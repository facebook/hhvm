<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x, true); }

//////////////////////////////////////////////////////////////////////

function test_icu_match() {
  // Test subject strings.
  $subject = "\xd7\x96\xf0\x90\xa4\x85". " PHP is a scripting language. " .
             "\xef\xba\xb0\xef\xbb\xb3";
  $subject_32 =
    "\xf0\x90\xa4\x85\xf0\x90\xa4\x85\xf0\x90\xa4\x85\xf0\x90\xa4\x85" .
    "\xf0\x90\xa4\x85\xf0\x90\xa4\x85";
  $subject_en = "this is an english string";
  // "this is a hebrew string"
  $subject_he =
    "\xd7\x96\xd7\x94\x20" .
    "\xd7\x94\xd7\x95\xd7\x90\x20\xd7\x9e\xd7\x97\xd7\xa8\xd7\x95\xd7" .
    "\x96\xd7\xaa\x20\xd7\xa2\xd7\x91\xd7\xa8\xd7\x99\xd7\xaa";
  // "this is an arabic string"
  $subject_ar =
    "\xef\xbb\xa9\xef".
    "\xba\xab\xef\xba\x8d\x20\xef\xbb\xa9\xef\xbb\xad\x20\xef\xba\x8e".
    "\xef\xbb\xa0\xef\xbb\xa8\xef\xba\xbb\x20\xef\xba\x8d\xef\xbb\xba".
    "\xef\xbb\xa8\xef\xba\xa0\xef\xbb\xa0\xef\xbb\xb3\xef\xba\xb0\xef".
    "\xbb\xb3";
  // "this is a hebrew string"
  $subject_mixed =
    "this is a ".
    "\xd7\xa2\xd7\x91\xd7\xa8\xd7\x99\xd7\xaa"
    ." string";

  // Test basic regex parsing functionality.
  VERIFY(icu_match("scripting", $subject) != false);
  VERIFY(icu_match("php", $subject) == false);
  VERIFY(icu_match("(\\bPHP\\b)", $subject) != false);
  VERIFY(icu_match("(\\bPHP\\b))", $subject) == false);

  // Test returning matches functionality.
  VERIFY(icu_match("(PHP) is", $subject, $matches) != false);
  VS(print_r($matches, true),
    "Array\n".
    "(\n".
    "    [0] => PHP is\n".
    "    [1] => PHP\n".
    ")\n");
  VERIFY(icu_match("is (a)", $subject, $matches,
                     UREGEX_OFFSET_CAPTURE) != false);
  VS(print_r($matches, true),
     "Array\n".
     "(\n".
     "    [0] => Array\n".
     "        (\n".
     "            [0] => is a\n".
     "            [1] => 7\n".
     "        )\n".
     "\n".
     "    [1] => Array\n".
     "        (\n".
     "            [0] => a\n".
     "            [1] => 10\n".
     "        )\n".
     "\n".
     ")\n");
  VERIFY(icu_match("\\. \xef\xba\xb0", $subject, $matches,
                     UREGEX_OFFSET_CAPTURE) != false);
  VS(print_r($matches, true),
    "Array\n".
    "(\n".
    "    [0] => Array\n".
    "        (\n".
    "            [0] => . \xef\xba\xb0\n".
    "            [1] => 30\n".
    "        )\n".
    "\n".
    ")\n");
  $junk1="\xef\xbb\xa9\xef\xbb\xad";
  $junk2="\xef\xba\x8e\xef\xbb\xa0\xef\xbb\xa8\xef\xba\xbb";
  VERIFY(icu_match("$junk1 ($junk2)",
                     $subject_ar, $matches, UREGEX_OFFSET_CAPTURE) != false);
  VS(print_r($matches, true),
    "Array\n".
    "(\n".
    "    [0] => Array\n".
    "        (\n".
    "            [0] => $junk1 $junk2\n".
    "            [1] => 4\n".
    "        )\n".
    "\n".
    "    [1] => Array\n".
    "        (\n".
    "            [0] => $junk2\n".
    "            [1] => 7\n".
    "        )\n".
    "\n".
    ")\n");

  // Test match for 32-bit code points.
  VERIFY(icu_match(".*", $subject_32, $matches) != false);
  $expected="\xf0\x90\xa4\x85\xf0\x90\xa4\x85\xf0\x90\xa4".
    "\x85\xf0\x90\xa4\x85\xf0\x90\xa4\x85\xf0\x90\xa4\x85";
  VS(print_r($matches, true),
    "Array\n".
    "(\n".
    "    [0] => $expected\n".
    ")\n");

  // Test regex caching functionality.
  VERIFY(icu_match("(php)", $subject, $ignore, UREGEX_CASE_INSENSITIVE) != false);
  VERIFY(icu_match("(php)", $subject) == false);

  // Test ICU specific (ie bidi) functionality.
  $pattern_ltr = "\\p{Bidi_Class=Left_To_Right}";
  $pattern_rtl = "\\p{Bidi_Class=Right_To_Left}";
  $pattern_arl = "\\p{Bidi_Class=Arabic_Letter}";

  VERIFY(icu_match($pattern_ltr, $subject_en) != false);
  VERIFY(icu_match($pattern_rtl, $subject_en) == false);

  VERIFY(icu_match($pattern_ltr, $subject_he) == false);
  VERIFY(icu_match($pattern_rtl, $subject_he) != false);
  VERIFY(icu_match($pattern_arl, $subject_he) == false);

  VERIFY(icu_match($pattern_ltr, $subject_ar) == false);
  VERIFY(icu_match($pattern_rtl, $subject_ar) == false);
  VERIFY(icu_match($pattern_arl, $subject_ar) != false);

  VERIFY(icu_match($pattern_ltr, $subject_mixed) != false);
  VERIFY(icu_match($pattern_rtl, $subject_mixed) != false);
}

// Test string lifted from tests/intl/utf8.h
function test_icu_transliterate() {
  $input_ru = "\xd1\x84\xd0\xb5\xd0\xb9\xd1".
           "\x81\xd0\xb1\xd1\x83\xc5\x93\xd0\xba";
  $output_ru = icu_transliterate($input_ru, false);
  // Note: different than php test ('y' -> 'j')
  VERIFY($output_ru == "fejsbu\xc5\x93k");

  // Verify that removing accents works.
  $input_de = "Ich m\xc3\xb6".
                           "chte \xc3\xbc".
                           "berzeugend ".
                            "oder \xc3\xa4hnliche sein";
  $output_de = icu_transliterate($input_de, true);
  VERIFY($output_de == "Ich mochte uberzeugend oder ahnliche sein");

  // Verify that keeping accents works.
  VERIFY(icu_transliterate($input_de, false) == $input_de);

  // Check a non-Latin language.
  $input_zh = "\xe5\x9b\x9b".
                           "\xe5\x8d\x81\xe5\x9b\x9b\xe7".
                           "\x9f\xb3\xe7\x8d\x85\xe5\xad\x90";
  $output_zh = icu_transliterate($input_zh, true);
  VERIFY($output_zh == "si shi si shi shi zi");
}


function test_icu_tokenize() {
  $input_eng = "Hello World";
  $output_eng = icu_tokenize($input_eng);

  VS(print_r($output_eng, true),
     "Array\n".
     "(\n".
     "    [0] => _B_\n".
     "    [1] => hello\n".
     "    [2] => world\n".
     "    [3] => _E_\n".
     ")\n"
    );
  $input_long = "Hello! You are visitor #1234 to ".
                            "http://www.facebook.com! ".
                            "<3 How are you today (6/14/2011),".
                            " hello@world.com?";

  $output_long = icu_tokenize($input_long);

  VS(print_r($output_long, true),
     "Array\n".
     "(\n".
     "    [0] => _B_\n".
     "    [1] => hello\n".
     "    [2] => !\n".
     "    [3] => you\n".
     "    [4] => are\n".
     "    [5] => visitor\n".
     "    [6] => #\n".
     "    [7] => XXXX\n".
     "    [8] => to\n".
     "    [9] => TOKEN_URL\n".
     "    [10] => !\n".
     "    [11] => TOKEN_HEART\n".
     "    [12] => how\n".
     "    [13] => are\n".
     "    [14] => you\n".
     "    [15] => today\n".
     "    [16] => (\n".
     "    [17] => TOKEN_DATE\n".
     "    [18] => )\n".
     "    [19] => ,\n".
     "    [20] => TOKEN_EMAIL\n".
     "    [21] => ?\n".
     "    [22] => _E_\n".
     ")\n"
    );

  $input_de = "Ich mÃ¶chte Ã¼berzeugend oder Ã¤hnliche sein";
  $output_de = icu_tokenize($input_de);

  VS(print_r($output_de, true),
     "Array\n".
     "(\n".
     "    [0] => _B_\n".
     "    [1] => ich\n".
     "    [2] => mã\n".
     "    [3] => ¶\n".
     "    [4] => chte\n".
     "    [5] => ã\n".
     "    [6] => ¼\n".
     "    [7] => berzeugend\n".
     "    [8] => oder\n".
     "    [9] => ã\n".
     "    [10] => ¤\n".
     "    [11] => hnliche\n".
     "    [12] => sein\n".
     "    [13] => _E_\n".
     ")\n");
}

test_icu_match();
test_icu_transliterate();
test_icu_tokenize();
