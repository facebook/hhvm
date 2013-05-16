<?php
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is 
                specified, break the string down into chunks each 
                split_length characters long. 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

echo "*** Testing str_split() : error conditions ***\n";

// Zero arguments
echo "-- Testing str_split() function with Zero arguments --\n";
var_dump( str_split() );

//Test str_split with one more than the expected number of arguments
echo "-- Testing str_split() function with more than expected no. of arguments --\n";
$str = 'This is error testcase';
$split_length = 4;
$extra_arg = 10;
var_dump( str_split( $str, $split_length, $extra_arg) );

echo "Done"
?>