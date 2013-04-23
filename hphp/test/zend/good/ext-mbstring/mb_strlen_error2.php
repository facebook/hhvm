<?php
/* Prototype  : int mb_strlen(string $str [, string $encoding])
 * Description: Get character numbers of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test mb_strlen when passed an unknown encoding
 */

echo "*** Testing mb_strlen() : error ***\n";

$string = 'abcdef';

$encoding = 'unknown-encoding';

var_dump(mb_strlen($string, $encoding));

echo "Done";
?>