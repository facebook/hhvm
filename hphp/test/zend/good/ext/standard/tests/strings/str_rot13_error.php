<?php
/* Prototype  : string str_rot13  ( string $str  )
 * Description: Perform the rot13 transform on a string
 * Source code: ext/standard/string.c
*/
echo "*** Testing str_rot13() : error conditions ***\n";

echo "-- Testing str_rot13() function with Zero arguments --\n";
var_dump( str_rot13() );

echo "\n\n-- Testing str_rot13() function with more than expected no. of arguments --\n";
$str = "str_rot13() tests starting";
$extra_arg = 10;
var_dump( str_rot13( $str, $extra_arg) );
?>
===DONE===
