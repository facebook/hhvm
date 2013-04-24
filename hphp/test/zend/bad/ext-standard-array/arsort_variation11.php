<?php
/* Prototype  : bool arsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array and maintain index association. 
                Elements will be arranged from highest to lowest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing arsort() by providing mixed value array for $array argument with following flag values.
 * 1.flag value as default
 * 2.SORT_REGULAR - compare items normally
*/

echo "*** Testing arsort() : usage variations ***\n";

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

echo "\n-- Testing arsort() by supplying mixed value array, 'flag' value is default --\n";
$temp_array = $mixed_values;
var_dump( arsort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing arsort() by supplying mixed value array, 'flag' value is SORT_REGULAR --\n";
$temp_array = $mixed_values;
var_dump( arsort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
?>