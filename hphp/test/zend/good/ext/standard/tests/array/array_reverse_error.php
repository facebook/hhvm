<?php
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed 
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_reverse() : error conditions ***\n";

// zero arguments
echo "\n-- Testing array_reverse() function with Zero arguments --\n";
var_dump( array_reverse() );

// more than the expected number of arguments
echo "\n-- Testing array_diff() function with more than expected no. of arguments --\n";
$array = array(1, 2, 3, 4, 5, 6);
$extra_arg = 10;
var_dump( array_reverse($array, true, $extra_arg) );
var_dump( array_reverse($array, false, $extra_arg) );

echo "Done";
?>