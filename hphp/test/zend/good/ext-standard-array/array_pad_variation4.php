<?php
/* Prototype  : array array_pad(array $input, int $pad_size, mixed $pad_value)
 * Description: Returns a copy of input array padded with pad_value to size pad_size
 * Source code: ext/standard/array.c
*/

/*
* Passing binary values to $pad_value argument and testing whether
* array_pad() behaves in an expected way with the other arguments passed to the function.
* The $input and $pad_size arguments passed are fixed values.
*/

echo "*** Testing array_pad() : Passing binary values to \$pad_value argument ***\n";

// initialize the $input and $pad_size argument
$input = array(1, 2, 3);
$pad_size = 6;

// initialize $pad_value with reference variable
$binary = b"hello";

var_dump( array_pad($input, $pad_size, $binary) );  // positive 'pad_size'
var_dump( array_pad($input, -$pad_size, $binary) );  // negative 'pad_size'

echo "Done";
?>