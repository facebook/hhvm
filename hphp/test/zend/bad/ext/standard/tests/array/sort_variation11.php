<?php
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing sort() by providing mixed value array for $array argument with following flag values.
 * flag value as defualt
 * SORT_REGULAR - compare items normally
*/

echo "*** Testing sort() : usage variations ***\n";

// mixed value array
$mixed_values = array (
  array(), 
  array(array(33, -5, 6), array(11), array(22, -55), array() ),
  -4, "4", 4.00, "b", "5", -2, -2.0, -2.98989, "-.9", "True", "",
  NULL, "ab", "abcd", 0.0, -0, "abcd\x00abcd\x00abcd", '', true, false
);

echo "\n-- Testing sort() by supplying mixed value array, 'flag' value is defualt --\n";
$temp_array = $mixed_values;
var_dump(sort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing sort() by supplying mixed value array, 'flag' value is SORT_REGULAR --\n";
$temp_array = $mixed_values;
var_dump(sort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
?>