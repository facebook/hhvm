<?php
/* Prototype  : mixed prev(array $array_arg)
 * Description: Move array argument's internal pointer to the previous element and return it 
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to prev() to test behaviour
 */

echo "*** Testing prev() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing prev() function with Zero arguments --\n";
var_dump( prev() );

//Test prev with one more than the expected number of arguments
echo "\n-- Testing prev() function with more than expected no. of arguments --\n";
$array_arg = array(1, 2);
$extra_arg = 10;
var_dump( prev($array_arg, $extra_arg) );
?>
===DONE===