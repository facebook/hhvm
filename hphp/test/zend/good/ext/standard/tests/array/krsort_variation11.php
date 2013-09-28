<?php
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation
 * Source code: ext/standard/array.c
*/

/*
 * testing krsort() by providing array of boolean values for $array argument with following flag values:
 *  1.flag value as defualt
 *  2.SORT_REGULAR - compare items normally
*/

echo "*** Testing krsort() : usage variations ***\n";

// bool value array
$bool_values = array (true => true, false => false, TRUE => TRUE, FALSE => FALSE);

echo "\n-- Testing krsort() by supplying boolean value array, 'flag' value is defualt --\n";
$temp_array = $bool_values;
var_dump(krsort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying boolean value array, 'flag' value is SORT_REGULAR --\n";
$temp_array = $bool_values;
var_dump(krsort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying boolean value array, 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $bool_values;
var_dump(krsort($temp_array, SORT_NUMERIC) );
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying boolean value array, 'flag' value is SORT_STRING --\n";
$temp_array = $bool_values;
var_dump(krsort($temp_array, SORT_STRING) );
var_dump($temp_array);

echo "Done\n";
?>