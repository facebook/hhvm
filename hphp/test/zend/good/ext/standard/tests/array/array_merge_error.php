<?php
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to array_merge() to test behaviour
 */

echo "*** Testing array_merge() : error conditions ***\n";

// Testing array_merge with zero arguments
echo "\n-- Testing array_merge() function with less than expected no. of arguments --\n";
$arr1 = array(1, 2);
try { var_dump( array_merge() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
?>
