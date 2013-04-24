<?php
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array 
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_unshift() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_unshift() function with Zero arguments --\n";
var_dump( array_unshift() );

// Testing array_unshift with one less than the expected number of arguments
echo "\n-- Testing array_unshift() function with less than expected no. of arguments --\n";
$array = array(1, 2);
var_dump( array_unshift($array) );
echo "Done";
?>