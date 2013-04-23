<?php
/* Prototype  : string iconv_substr(string str, int offset, [int length, string charset])
 * Description: Returns part of a string 
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass an unknown encoding to iconv_substr() to test behaviour
 */

echo "*** Testing iconv_substr() : error conditions ***\n";

$str = 'Hello, world';
$start = 1;
$length = 5;
$encoding = 'unknown-encoding';

var_dump( iconv_substr($str, $start, $length, $encoding));

echo "Done";
?>