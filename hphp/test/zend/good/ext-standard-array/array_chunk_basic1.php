<?php
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 *              Chunks an array into size  large chunks.
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_chunk() : basic functionality ***\n";
$size = 2;

$input_arrays = array (
  // array with default keys - numeric values
  array(1, 2, 3, 4, 5),

  // array with default keys - string values
  array('value1', "value2", "value3"),

  // associative arrays - key as string
  array('key1' => 1, "key2" => 2, "key3" => 3),
 
  // associative arrays - key as numeric
  array(1 => 'one', 2 => "two", 3 => "three"),

  // array containing elements with/witout keys 
  array(1 => 'one','two', 3 => 'three', 4, "five" => 5)

); 

$count = 1;
// loop through each element of the array for input
foreach ($input_arrays as $input_array){ 
  echo "\n-- Iteration $count --\n";  
  var_dump( array_chunk($input_array, $size) );
  $count++;
}

echo "Done"
?>