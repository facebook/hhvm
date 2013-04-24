<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass rsort() an array of different data types to test behaviour
 */

echo "*** Testing rsort() : variation ***\n";

// mixed value array
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

echo "\n-- Sort flag = default --\n";
$temp_array = $mixed_values;
var_dump(rsort($temp_array) );
var_dump($temp_array);

echo "\n-- Sort flag = SORT_REGULAR --\n";
$temp_array = $mixed_values;
var_dump(rsort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done";
?>