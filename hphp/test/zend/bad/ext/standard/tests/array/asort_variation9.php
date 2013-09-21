<?php
/* Prototype  : bool asort ( array &$array [, int $sort_flags] )
 * Description: Sort an array and maintain index association. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * Testing asort() by providing arrays  with key values for $array argument with following flag values.
 * 1.flag value as default
 * 2.SORT_REGULAR - compare items normally
 */

echo "*** Testing asort() : usage variations ***\n";

// list of arrays with/without key values
$various_arrays = array (
  array(5 => 55,  66, 22, 33, 11),
  array ("a" => "orange",  "banana", "c" => "apple"),
  array(1, 2, 3, 4, 5, 6),
  array("first", 5 => "second", "third"),
  array(1, 1, 8 => 1,  4 => 1, 19, 3 => 13),
  array('bar' => 'baz', "foo" => 1),
  array('a'=>1,'b'=>array('e'=>2,'f'=>3),'c'=>array('g'=>4),'d'=>5),
);

$count = 1;
echo "\n-- Testing asort() by supplying various arrays with key values --\n";

// loop through to test asort() with different arrays, 
// to test the new keys for the elements in the sorted array 
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With default sort_flag -\n";
  $temp_array = $array;
  var_dump(asort($temp_array) );
  var_dump($temp_array);

  echo "- Sort_flag = SORT_REGULAR -\n";
  $temp_array = $array;
  var_dump(asort($temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done\n";
?>