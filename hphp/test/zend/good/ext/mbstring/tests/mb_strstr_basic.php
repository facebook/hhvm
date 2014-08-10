<?php
/* Prototype  : string mb_strstr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds first occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_strstr() : basic functionality ***\n";

mb_internal_encoding('UTF-8');

$string_ascii = b'abc def';
//Japanese string in UTF-8
$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

echo "\n-- ASCII string: needle exists --\n";
var_dump(bin2hex(mb_strstr($string_ascii, b'd', false, 'ISO-8859-1')));
var_dump(bin2hex(mb_strstr($string_ascii, b'd')));
var_dump(bin2hex(mb_strstr($string_ascii, b'd', true)));


echo "\n-- ASCII string: needle doesn't exist --\n";
var_dump(mb_strstr($string_ascii, b'123'));

echo "\n-- Multibyte string: needle exists --\n";
$needle1 = base64_decode('5pel5pys6Kqe');
var_dump(bin2hex(mb_strstr($string_mb, $needle1)));
var_dump(bin2hex(mb_strstr($string_mb, $needle1, false, 'utf-8')));
var_dump(bin2hex(mb_strstr($string_mb, $needle1, true)));


echo "\n-- Multibyte string: needle doesn't exist --\n";
$needle2 = base64_decode('44GT44KT44Gr44Gh44Gv44CB5LiW55WM');
var_dump(mb_strstr($string_mb, $needle2));

?>
===DONE===
