<?php
/* Prototype  : int mb_strpos(string $haystack, string $needle [, int $offset [, string $encoding]])
 * Description: Find position of first occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass an unknown encoding to mb_strpos() to test behaviour
 */

echo "*** Testing mb_strpos() : error conditions ***\n";
$haystack = 'Hello, world';
$needle = 'world';
$offset = 2;
$encoding = 'unknown-encoding';

var_dump( mb_strpos($haystack, $needle, $offset, $encoding) );

echo "Done";
?>