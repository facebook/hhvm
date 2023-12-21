<?hh
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 *            : Chunks an array into size large chunks
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_chunk() function with following conditions 
 *   1. array without elements
 *   2. associative array with duplicate keys
 *   3. array with one element 
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : usage variations ***\n";

// input array
$input_arrays = dict[

  // array without elements
  "array1" => vec[],

  // array with one element
  "array2" => vec[1],
  
  // associative array with duplicate keys
  "array3" => dict["a" => 1, "b" => 2, "c" => 3, "a" => 4, "d" => 5]

];

$size = 2;
$count = 1;

echo "\n-- Testing array_chunk() by supplying various arrays --\n";

// loop through the array for 'array' argument
foreach ($input_arrays as $input_array){
  echo "\n-- Iteration $count --\n";
  var_dump( array_chunk($input_array, $size) );
  var_dump( array_chunk($input_array, $size, true) );
  var_dump( array_chunk($input_array, $size, false) );
  $count++;
}

echo "Done";
}
