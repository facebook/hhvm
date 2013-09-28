<?php
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 *            : Chunks an array into size large chunks
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_chunk() function with following conditions
 *   1. -ve size value
 *   2. size value is more than the no. of elements in the input array
 *   3. size value is zero
 *   4. Decimal size value
*/

echo "*** Testing array_chunk() : usage variations ***\n";

// input array
$input_array = array(1, 2, 3);

// different magnitude's
$sizes = array(-1, count($input_array) + 1, 0, 1.5);

// loop through the array for size argument
foreach ($sizes as $size){
  echo "\n-- Testing array_chunk() when size = $size --\n";
  var_dump( array_chunk($input_array, $size) );
  var_dump( array_chunk($input_array, $size, true) );
  var_dump( array_chunk($input_array, $size, false) );
}
echo "Done";
?>