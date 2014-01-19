<?php
/* Prototype  : string mb_strrichr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds the last occurrence of a character in a string within another, case insensitive 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_strrichr() : error conditions ***\n";


echo "\n-- Testing mb_strrichr() with unknown encoding --\n";
$haystack = b'Hello, world';
$needle = b'world';
$encoding = 'unknown-encoding';
$part = true;
var_dump( mb_strrichr($haystack, $needle, $part, $encoding) );

?>
===DONE===