<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass rsort() an array of hexadecimal values to test behaviour
 */

echo "*** Testing rsort() : variation ***\n";

// an array contains unsorted hexadecimal values  
$unsorted_hex_array = array(0x1AB, 0xFFF, 0xF, 0xFF, 0x2AA, 0xBB, 0x1ab, 0xff, -0xFF, 0, -0x2aa);

echo "\n-- 'flag' value is defualt  --\n";
$temp_array = $unsorted_hex_array;
var_dump(rsort($temp_array) );
var_dump($temp_array);

echo "\n-- 'flag' value is SORT_REGULAR  --\n";
$temp_array = $unsorted_hex_array;
var_dump(rsort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "\n-- 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $unsorted_hex_array;
var_dump(rsort($temp_array, SORT_NUMERIC) );
var_dump($temp_array);

echo "Done";
?>
