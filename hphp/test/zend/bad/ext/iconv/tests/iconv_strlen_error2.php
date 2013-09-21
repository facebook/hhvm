<?php
/* Prototype  : int iconv_strlen(string str [, string charset])
 * Description: Get character numbers of a string 
 * Source code: ext/iconv/iconv.c
 */

/*
 * Test iconv_strlen when passed an unknown encoding
 */

echo "*** Testing iconv_strlen() : error ***\n";

$string = 'abcdef';

$encoding = 'unknown-encoding';

var_dump(iconv_strlen($string, $encoding));

?>
===DONE===