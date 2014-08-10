<?php
/* Prototype  : mixed end(array $array_arg)
 * Description: Advances array argument's internal pointer to the last element and return it 
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to end() to test behaviour
 */

echo "*** Testing end() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing end() function with Zero arguments --\n";
var_dump( end() );

//Test end with one more than the expected number of arguments
echo "\n-- Testing end() function with more than expected no. of arguments --\n";
$array_arg = array(1, 2);
$extra_arg = 10;
var_dump( end($array_arg, $extra_arg) );
?>
===DONE===
