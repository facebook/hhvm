<?php
/* Prototype  : string mb_stristr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds first occurrence of a string within another, case insensitive 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_stristr() : basic functionality ***\n";

mb_internal_encoding('UTF-8');

//ascii mixed case, multiple needles
$string_ascii = b'abcDef zBcDyx';
$needle_ascii_upper = b"BCD";
$needle_ascii_mixed = b"bCd";
$needle_ascii_lower = b"bcd";

//Greek string in mixed case UTF-8 with multiple needles
$string_mb = base64_decode('zrrOu868zr3Ovs6fzqDOoSDOus67zpzOnc6+zr/OoA==');
$needle_mb_upper = base64_decode('zpzOnc6ezp8=');
$needle_mb_lower = base64_decode('zrzOvc6+zr8=');
$needle_mb_mixed = base64_decode('zpzOnc6+zr8=');

echo "\n-- ASCII string: needle exists --\n";
var_dump(bin2hex(mb_stristr($string_ascii, $needle_ascii_upper, false)));
var_dump(bin2hex(mb_stristr($string_ascii, $needle_ascii_upper, true)));
var_dump(bin2hex(mb_stristr($string_ascii, $needle_ascii_lower, false)));
var_dump(bin2hex(mb_stristr($string_ascii, $needle_ascii_lower, true)));
var_dump(bin2hex(mb_stristr($string_ascii, $needle_ascii_mixed, false)));
var_dump(bin2hex(mb_stristr($string_ascii, $needle_ascii_mixed, true)));


echo "\n-- Multibyte string: needle exists --\n";
var_dump(bin2hex(mb_stristr($string_mb, $needle_mb_upper, false)));
var_dump(bin2hex(mb_stristr($string_mb, $needle_mb_upper, true)));
var_dump(bin2hex(mb_stristr($string_mb, $needle_mb_lower, false)));
var_dump(bin2hex(mb_stristr($string_mb, $needle_mb_lower, true)));
var_dump(bin2hex(mb_stristr($string_mb, $needle_mb_mixed, false)));
var_dump(bin2hex(mb_stristr($string_mb, $needle_mb_mixed, true)));

?>
===DONE===