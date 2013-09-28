<?php
/* Prototype  : array array_intersect_assoc(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments. 
 * Keys are used to do more restrictive check 
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_intersect_assoc() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_intersect_assoc() function with Zero arguments --\n";
var_dump( array_intersect_assoc() );

// Testing array_intersect_assoc with one less than the expected number of arguments
echo "\n-- Testing array_intersect_assoc() function with less than expected no. of arguments --\n";
$arr1 = array(1, 2);
var_dump( array_intersect_assoc($arr1) );

echo "Done";
?>