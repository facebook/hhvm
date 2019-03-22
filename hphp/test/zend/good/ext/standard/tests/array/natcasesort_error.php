<?php
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to natcasesort() to test behaviour
 */

echo "*** Testing natcasesort() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing natcasesort() function with Zero arguments --\n";
try { var_dump( natcasesort() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Test natcasesort with one more than the expected number of arguments
echo "\n-- Testing natcasesort() function with more than expected no. of arguments --\n";
$array_arg = array(1, 2);
$extra_arg = 10;
try { var_dump( natcasesort(&$array_arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
