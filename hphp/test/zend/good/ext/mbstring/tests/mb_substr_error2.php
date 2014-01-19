<?php
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass an unknown encoding to mb_substr() to test behaviour
 */

echo "*** Testing mb_substr() : error conditions ***\n";

$str = 'Hello, world';
$start = 1;
$length = 5;
$encoding = 'unknown-encoding';

var_dump( mb_substr($str, $start, $length, $encoding));

echo "Done";
?>