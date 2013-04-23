<?php
/* Prototype  : proto string mb_regex_encoding([string encoding])
 * Description: Returns the current encoding for regex as a string. 
 * Source code: ext/mbstring/php_mbregex.c 
 */

/*
 * Test Basic functionality of mb_regex_encoding
 */

echo "*** Testing mb_regex_encoding() : basic functionality ***\n";

var_dump(mb_regex_encoding());

var_dump(mb_regex_encoding('UTF-8'));

var_dump(mb_regex_encoding());


echo "Done";
?>