<?php
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */

echo "*** Testing array_multisort() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_multisort() function with Zero arguments --\n";
var_dump( array_multisort() );

echo "\n-- Testing array_multisort() function with repeated flags --\n";
$ar1 = array(1);
var_dump( array_multisort($ar1, SORT_ASC, SORT_ASC) );

echo "\n-- Testing array_multisort() function with repeated flags --\n";
$ar1 = array(1);
var_dump( array_multisort($ar1, SORT_STRING, SORT_NUMERIC) );

?>
===DONE===