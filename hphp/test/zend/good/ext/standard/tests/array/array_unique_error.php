<?php
/* Prototype  : array array_unique(array $input)
 * Description: Removes duplicate values from array 
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_unique() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_unique() function with zero arguments --\n";
var_dump( array_unique() );

//Test array_unique with one more than the expected number of arguments
echo "\n-- Testing array_unique() function with more than expected no. of arguments --\n";
$input = array(1, 2);
$extra_arg = 10;
var_dump( array_unique($input, SORT_NUMERIC, $extra_arg) );

echo "Done";
?>