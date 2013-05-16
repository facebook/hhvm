<?php
/* Prototype  : string mb_strrchr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds the last occurrence of a character in a string within another 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_strrchr() : error conditions ***\n";


echo "\n-- Testing mb_strrchr() with unknown encoding --\n";
$haystack = b'Hello, world';
$needle = b'world';
$encoding = 'unknown-encoding';
$part = true;
var_dump( mb_strrchr($haystack, $needle, $part, $encoding) );

?>
===DONE===