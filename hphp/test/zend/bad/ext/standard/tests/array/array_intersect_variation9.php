<?php
/* Prototype  : array array_intersect(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments 
 * Source code: ext/standard/array.c
*/

/*
* Testing the behavior of array_intersect() by passing 2-D arrays
* to both $arr1 and $arr2 argument. 
* Optional argument takes the same value as that of $arr1
*/

echo "*** Testing array_intersect() : passing two dimensional array to both \$arr1 and \$arr2 arguments ***\n";

// two dimensional arrays for $arr1 and $arr2 argument
$arr1 = array (
  
  // arrays with default keys
  array(1, 2, "hello", 'world'),
  array(1, 2, 3, 4),

  // arrays with explicit keys
  array(1 => "one", 2 => "two", 3 => "three"),
  array("ten" => 10, "twenty" => 20.00, "thirty" => 30)
);  

$arr2 = array (
  array(1, 2, 3, 4),
  array(1 => "one", 2 => "two", 3 => "three")
);

/* Passing the entire array as argument to $arr1 and $arr2 */
// Calling array_intersect() with default arguments
echo "-- Passing the entire 2-D array to \$arr1 and \$arr2 --\n";
echo "- With default arguments -\n";
var_dump( array_intersect($arr1, $arr2) );

// Calling array_intersect() with more arguments
// additional argument passed is the same as $arr1
echo "- With more arguments -\n";
var_dump( array_intersect($arr1, $arr2, $arr1) );

/* Passing the sub-array as argument to $arr1 and $arr2 */
// Calling array_intersect() with default arguments
echo "-- Passing the sub-array to \$arr1 and \$arr2 --\n";
echo "- With default arguments -\n";
var_dump( array_intersect($arr1[0], $arr2[0]) );

// Calling array_intersect() with more arguments
// additional argument passed is the same as $arr1
echo "- With more arguments -\n";
var_dump( array_intersect($arr1[0], $arr2[0], $arr1[0]) );

echo "Done";
?>