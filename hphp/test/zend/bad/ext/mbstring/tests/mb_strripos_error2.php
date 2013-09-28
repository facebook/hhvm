<?php
/* Prototype  : int mb_strripos(string haystack, string needle [, int offset [, string encoding]])
 * Description: Finds position of last occurrence of a string within another, case insensitive 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

/*
 * Pass an unknown encoding to mb_strripos() to test behaviour
 */

echo "*** Testing mb_strripos() : error conditions ***\n";
$haystack = b'Hello, world';
$needle = b'world';
$offset = 2;
$encoding = 'unknown-encoding';

var_dump( mb_strripos($haystack, $needle, $offset, $encoding) );

echo "Done";
?>