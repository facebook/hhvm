<?php
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass an array containing sub-arrays, ints, floats, strings, boolean, null 
 * and escape characters to test how natcasesort() re-orders it
 */

echo "*** Testing natcasesort() : usage variation ***\n";

$mixed_values = array (
  array(), 
  array( array(33, -5, 6), 
         array(11), 
         array(22, -55), 
         array() 
       ),
  -4, "4", 4.00, "b", "5", -2, -2.0, -2.98989, "-.9", "True", "",
  NULL, "ab", "abcd", 0.0, -0, "abcd\x00abcd\x00abcd", '', true, false
);
// suppress errors as is generating a lot of "array to string" notices
var_dump( @natcasesort($mixed_values) );

var_dump($mixed_values);

echo "Done";
?>
