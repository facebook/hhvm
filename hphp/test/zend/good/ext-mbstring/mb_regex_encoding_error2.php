<?php
/* Prototype  : string mb_regex_encoding([string $encoding])
 * Description: Returns the current encoding for regex as a string. 
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Pass mb_regex_encoding an unknown type of encoding
 */

echo "*** Testing mb_regex_encoding() : error conditions ***\n";

var_dump(mb_regex_encoding('unknown'));


echo "Done";
?>