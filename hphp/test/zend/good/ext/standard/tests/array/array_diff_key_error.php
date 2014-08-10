<?php
/* Prototype  : array array_diff_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments. 
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_diff_key() : error conditions ***\n";

// Initialise the variables
$array1 = array('blue' => 1, 'red' => 2, 'green' => 3, 'purple' => 4);

// Testing array_diff_key with one less than the expected number of arguments
echo "\n-- Testing array_diff_key() function with less than expected no. of arguments --\n";
var_dump( array_diff_key($array1) );

// Testing array_diff_key with no arguments
echo "\n-- Testing array_diff_key() function with no arguments --\n";
var_dump( array_diff_key() );
?>
===DONE===
