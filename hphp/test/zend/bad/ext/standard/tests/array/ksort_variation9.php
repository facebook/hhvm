<?php
/* Prototype  : bool ksort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key, maintaining key to data correlation. 
 * Source code: ext/standard/array.c
*/

/*
 * Testing ksort() by providing arrays  with/without key values for $array argument with following flag values: 
 *  1.flag value as defualt
 *  2.SORT_REGULAR - compare items normally
 */

echo "*** Testing ksort() : usage variations ***\n";

// list of arrays with/without key values
$various_arrays = array (
  array(5 => 55,  66, 22, 33, 11),
  array ("a" => "orange",  "banana", "c" => "apple"),
  array(1, 2, 3, 4, 5, 6),
  array("first", 5 => "second", 1 => "third"),
  array(1, 1, 8 => 1,  4 => 1, 19, 3 => 13),
  array('bar' => 'baz', "foo" => 1),
  array('a' => 1,'b' => array('e' => 2,'f' => 3),'c' => array('g' => 4),'d' => 5),
);

$count = 1;
echo "\n-- Testing ksort() by supplying various arrays with/without key values --\n";

// loop through to test ksort() with different arrays, 
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With defualt sort flag -\n";
  $temp_array = $array;
  var_dump( ksort($temp_array) );
  var_dump($temp_array);

  echo "- Sort flag = SORT_REGULAR -\n";
  $temp_array = $array;
  var_dump( ksort($temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done\n";
?>