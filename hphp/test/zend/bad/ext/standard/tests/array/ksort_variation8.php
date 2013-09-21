<?php
/* Prototype  : bool ksort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key, maintaining key to data correlation. 
 * Source code: ext/standard/array.c
*/

/*
 * testing ksort() by providing array of mixed values for $array argument with following flag values:
 *  1.flag value as defualt
 *  2.SORT_REGULAR - compare items normally
*/

echo "*** Testing ksort() : usage variations ***\n";

// mixed value array with different key values
$mixed_values = array (
  "array1" => array(), 
  "array2" => array ( "sub_array[2,1]" => array(33,-5,6), "sub_array[2,2]" => array(11), 
                      "sub_array[2,3]" => array(22,-55), "sub_array[2,4]" => array() 
                    ),
  4 => 4, "4" => "4", 4.01 => 4.01, "b" => "b", "5" => "5", -2 => -2, -2.01 => -2.01, 
  -2.98989 => -2.98989, "-.9" => "-.9", "True" => "True", "" =>  "", NULL => NULL,
  "ab" => "ab", "abcd" => "abcd", 0.01 => 0.01, -0 => -0, '' => '' ,
  "abcd\x00abcd\x00abcd" => "abcd\x00abcd\x00abcd", 0.001 => 0.001
);

echo "\n-- Testing ksort() by supplying mixed value array, 'flag' value is defualt --\n";
$temp_array = $mixed_values;
var_dump( ksort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing ksort() by supplying mixed value array, 'flag' value is SORT_REGULAR --\n";
$temp_array = $mixed_values;
var_dump( ksort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
?>