<?php
/* Prototype  : bool arsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array and maintain index association. 
                Elements will be arranged from highest to lowest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing arsort() by providing different octal array for $array argument with following flag values
 * 1.flag value as defualt
 * 2.SORT_REGULAR - compare items normally
 * 3.SORT_NUMERIC - compare items numerically
*/

echo "*** Testing arsort() : usage variations ***\n";

// an array contains unsorted octal values
$unsorted_oct_array = array ( 
   01235 => 01235, 0321 => 0321, 0345 => 0345, 066 => 066, 0772 => 0772, 
   077 => 077, -066 => -066, -0345 => -0345, 0 => 0
);

echo "\n-- Testing arsort() by supplying octal value array, 'flag' value is defualt  --\n";
$temp_array = $unsorted_oct_array;
var_dump( arsort($temp_array) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing arsort() by supplying octal value array, 'flag' value is SORT_REGULAR  --\n";
$temp_array = $unsorted_oct_array;
var_dump( arsort($temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing arsort() by supplying octal value array, 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $unsorted_oct_array;
var_dump( arsort($temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump($temp_array);

echo "Done\n";
?>