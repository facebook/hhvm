<?php
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation. 
 * Source code: ext/standard/array.c
*/

/*
 * testing krsort() by providing array of octal values for $array argument
 * with following flag values:
 *  1.flag value as defualt
 *  2.SORT_REGULAR - compare items normally
 *  3.SORT_NUMERIC - compare items numerically
*/

echo "*** Testing krsort() : usage variations ***\n";

// an array containing unsorted octal values
$unsorted_oct_array = array (
  01235 => 01, 0321 => 02, 0345 => 03, 066 => 04, 0772 => 05,
  077 => 06, -066 => -01, -0345 => -02, 0 => 0
);

echo "\n-- Testing krsort() by supplying octal value array, 'flag' value is defualt  --\n";
$temp_array = $unsorted_oct_array;
var_dump( krsort($temp_array) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying octal value array, 'flag' value is SORT_REGULAR  --\n";
$temp_array = $unsorted_oct_array;
var_dump( krsort($temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying octal value array, 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $unsorted_oct_array;
var_dump( krsort($temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump($temp_array);

echo "Done\n";
?>