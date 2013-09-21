<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass rsort() associative arrays to test key re-assignment
 */

echo "*** Testing rsort() : variation ***\n";

// Associative arrays
$various_arrays = array(
	// numeric assoc. only array
	array(5 => 55, 6 => 66, 2 => 22, 3 => 33, 1 => 11),

	// two-dimensional assoc. and default key array
	array("fruits"  => array("a" => "orange", "b" => "banana", "c" => "apple"),
     	  "numbers" => array(1, 2, 3, 4, 5, 6),
     	  "holes"   => array("first", 5 => "second", "third")),

	// numeric assoc. and default key array
	array(1, 1, 8 => 1,  4 => 1, 19, 3 => 13),

	// mixed assoc. array
	array('bar' => 'baz', "foo" => 1),

	// assoc. only multi-dimensional array
	array('a' => 1,'b' => array('e' => 2,'f' => 3),'c' => array('g' => 4),'d' => 5),
);

$count = 1;

// loop through to test rsort() with different arrays, 
// to test the new keys for the elements in the sorted array 
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "-- Sort flag = default --\n";
  $temp_array = $array;
  var_dump(rsort($temp_array) );
  var_dump($temp_array);

  echo "-- Sort flag = SORT_REGULAR --\n";
  $temp_array = $array;
  var_dump(rsort($temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done";
?>