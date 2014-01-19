<?php
/* Prototype  : string mb_stristr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds first occurrence of a string within another, case insensitive 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_stristr() : error conditions ***\n";


echo "\n-- Testing mb_stristr() with unknown encoding --\n";
$haystack = b'Hello, world';
$needle = b'world';
$encoding = 'unknown-encoding';
$part = true;
var_dump( mb_stristr($haystack, $needle, $part, $encoding) );

?>
===DONE===