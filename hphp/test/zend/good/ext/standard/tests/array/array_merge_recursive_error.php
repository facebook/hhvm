<?php
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_merge_recursive() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_merge_recursive() function with Zero arguments --\n";
try { var_dump( array_merge_recursive() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
?>
