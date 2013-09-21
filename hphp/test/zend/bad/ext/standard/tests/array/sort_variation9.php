<?php
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * Testing sort() by providing arrays  with key values for $array argument
 * with following flag values.
 * 1.flag value as defualt
 * 2.SORT_REGULAR - compare items normally
 * To test the new keys for the elements in the sorted array.
 */

echo "*** Testing sort() : usage variations ***\n";

// list of arrays with key and values
$various_arrays = array(
  array(5 => 55, 6 => 66, 2 => 22, 3 => 33, 1 => 11),
  array ("fruits"  => array("a" => "orange", "b" => "banana", "c" => "apple"),
         "numbers" => array(1, 2, 3, 4, 5, 6),
         "holes"   => array("first", 5 => "second", "third")
        ),
  array(1, 1, 8 => 1,  4 => 1, 19, 3 => 13),
  array('bar' => 'baz', "foo" => 1),
  array('a' => 1,'b' => array('e' => 2,'f' => 3),'c' => array('g' => 4),'d' => 5),
);

$count = 1;
echo "\n-- Testing sort() by supplying various arrays with key values --\n";

// loop through to test sort() with different arrays, 
// to test the new keys for the elements in the sorted array 
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With Defualt sort flag -\n";
  $temp_array = $array;
  var_dump(sort($temp_array) );
  var_dump($temp_array);

  echo "- Sort flag = SORT_REGULAR -\n";
  $temp_array = $array;
  var_dump(sort($temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done\n";
?>