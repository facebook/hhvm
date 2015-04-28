<?php

// https://github.com/facebook/hhvm/issues/5010
// utf8_decode() targets latin1, therefore any character
// greater than U+00FF should come out as
// U+003F QUESTION MARK

// U+00DF LATIN SMALL LETTER SHARP S
$ss = "\u{DF}";
var_dump(urlencode($ss));
var_dump(urlencode(utf8_decode($ss)));
var_dump(json_encode(utf8_decode($ss)));

// U+044F CYRILLIC SMALL LETTER YA
$ya = "\u{44F}";
var_dump(urlencode($ya));
var_dump(urlencode(utf8_decode($ya)));
var_dump(json_encode(utf8_decode($ya)));

// Improperly formatted U+0041 LATIN CAPITAL LETTER A
$A = "\xC1\x81";
var_dump(urlencode($A));
var_dump(urlencode(utf8_decode($A)));
var_dump(json_encode(utf8_decode($A)));
