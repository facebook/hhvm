<?php
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length 
 * Source code: ext/standard/array.c
 */

/*
 * Pass an incorrect number of arguments to array_slice() to test behaviour
 */

echo "*** Testing array_slice() : error conditions ***\n";

//Test array_slice with one more than the expected number of arguments
echo "\n-- Testing array_slice() function with more than expected no. of arguments --\n";
$input = array(1, 2);
$offset = 10;
$length = 10;
$preserve_keys = true;
$extra_arg = 10;
var_dump( array_slice($input, $offset, $length, $preserve_keys, $extra_arg) );

// Testing array_slice with one less than the expected number of arguments
echo "\n-- Testing array_slice() function with less than expected no. of arguments --\n";
var_dump( array_slice($input) );

echo "Done";
?>