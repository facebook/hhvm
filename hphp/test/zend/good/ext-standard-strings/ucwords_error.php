<?php
/* Prototype  : string ucwords ( string $str )
 * Description: Uppercase the first character of each word in a string
 * Source code: ext/standard/string.c
*/

echo "*** Testing ucwords() : error conditions ***\n";

// Zero argument
echo "\n-- Testing ucwords() function with Zero arguments --\n";
var_dump( ucwords() );

// More than expected number of arguments
echo "\n-- Testing ucwords() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;

var_dump( ucwords($str, $extra_arg) );

// check if there were any changes made to $str
var_dump($str);

echo "Done\n";
?>