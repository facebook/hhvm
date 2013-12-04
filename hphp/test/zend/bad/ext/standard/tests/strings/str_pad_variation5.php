<?php
/* Prototype  : string str_pad  ( string $input  , int $pad_length  [, string $pad_string  [, int $pad_type  ]] )
 * Description: Pad a string to a certain length with another string
 * Source code: ext/standard/string.c
*/

/* Test str_pad() function: with unexpected inputs for '$pad_length' 
 *  and expected type for '$input'
*/

echo "*** Testing str_pad() function: with large value for for 'pad_length' argument ***\n";

//defining '$input' argument
$input = "Test string";
$pad_length = PHP_INT_MAX;
var_dump( str_pad($input, $pad_length) );

?>