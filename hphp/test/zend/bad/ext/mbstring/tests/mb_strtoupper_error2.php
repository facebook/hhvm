<?php
/* Prototype  : string mb_strtoupper(string $sourcestring [, string $encoding]
 * Description: Returns a uppercased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass an unknown encoding as $encoding argument to check behaviour of mbstrtoupper()
 */

echo "*** Testing mb_strtoupper() : error conditions ***\n";

$sourcestring = 'hello, world';
$encoding = 'unknown-encoding';

var_dump( mb_strtoupper($sourcestring, $encoding) );

echo "Done";
?>
