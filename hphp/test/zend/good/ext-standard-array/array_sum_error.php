<?php
/* Prototype  : mixed array_sum(array &input)
 * Description: Returns the sum of the array entries 
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_sum() : error conditions ***\n";

// Zero arguments
echo "-- Testing array_sum() function with zero arguments --\n";
var_dump( array_sum() );

// One more than the expected number of arguments
echo "-- Testing array_sum() function with more than expected no. of arguments --\n";
$input = array(1, 2, 3, 4);
$extra_arg = 10;
var_dump( array_sum($input, $extra_arg) );

echo "Done"
?>