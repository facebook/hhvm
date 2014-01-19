<?php
/* Prototype  : int mb_strrpos(string $haystack, string $needle [, int $offset [, string $encoding]])
 * Description: Find position of last occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test basic functionality of mb_strrpos()
 */

echo "*** Testing mb_strrpos() : basic ***\n";

mb_internal_encoding('UTF-8');

$string_ascii = b'This is an English string. 0123456789.';
//Japanese string in UTF-8
$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

echo "\n-- ASCII string 1 --\n";
var_dump(mb_strrpos($string_ascii, b'is', 4, 'ISO-8859-1'));

echo "\n-- ASCII string 2 --\n";
var_dump(mb_strrpos($string_ascii, b'hello, world'));

echo "\n-- Multibyte string 1 --\n";
$needle1 = base64_decode('44CC');
var_dump(mb_strrpos($string_mb, $needle1));

echo "\n-- Multibyte string 2 --\n";
$needle2 = base64_decode('44GT44KT44Gr44Gh44Gv44CB5LiW55WM');
var_dump(mb_strrpos($string_mb, $needle2));

echo "Done";
?>