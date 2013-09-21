<?php
/* Prototype  : int mb_substr_count(string $haystack, string $needle [, string $encoding])
 * Description: Count the number of substring occurrences 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test behaviour of mb_substr_count() function when passed an unknown encoding
 */

echo "*** Testing mb_substr_count() : error conditions ***\n";

$haystack = 'Hello, World!';
$needle = 'Hello';
$encoding = 'unknown-encoding';

echo "\n-- Testing mb_substr_count() function with an unknown encoding --\n";
var_dump(mb_substr_count($haystack, $needle, $encoding));

echo "Done";
?>