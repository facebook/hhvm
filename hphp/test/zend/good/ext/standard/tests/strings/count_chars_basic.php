<?php

/* Prototype  : mixed count_chars  ( string $string  [, int $mode  ] )
 * Description: Return information about characters used in a string
 * Source code: ext/standard/string.c
*/


echo "*** Testing count_chars() : basic functionality ***\n";

$string = "Return information about characters used in a string";

var_dump(count_chars($string));
var_dump(count_chars($string, 0));
var_dump(count_chars($string, 1));
var_dump(count_chars($string, 2));
var_dump(count_chars($string, 3));
var_dump(bin2hex(count_chars($string, 4)));


?>
===DONE===