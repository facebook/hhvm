<?php
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is 
                specified, break the string down into chunks each 
                split_length characters long. 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

echo "*** Testing str_split() : basic functionality ***\n";

// Initialise all required variables
$str = 'This is basic testcase';
$split_length = 5;

// Calling str_split() with all possible arguments
echo "-- With all possible arguments --\n";
var_dump( str_split($str,$split_length) );

// Calling str_split() with default arguments
echo "-- With split_length as default argument --\n";
var_dump( str_split($str) );

echo "Done"
?>