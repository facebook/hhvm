<?php
/* Prototype  : int iconv_strpos(string haystack, string needle [, int offset [, string charset]])
 * Description: Find position of first occurrence of a string within another 
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass an unknown encoding to iconv_strpos() to test behaviour
 */

echo "*** Testing iconv_strpos() : error conditions ***\n";
$haystack = 'Hello, world';
$needle = 'world';
$offset = 2;
$encoding = 'unknown-encoding';

var_dump( iconv_strpos($haystack, $needle, $offset, $encoding) );

echo "Done";
?>