<?hh
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
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : usage variations ***\n";

// input array
$input_array = vec[1, 2, 3];

// different magnitude's
$sizes = vec[-1, count($input_array) + 1, 0, 1.5];

// loop through the array for size argument
foreach ($sizes as $size){
  $size__str = (string)($size);
  echo "\n-- Testing array_chunk() when size = $size__str --\n";
  var_dump( array_chunk($input_array, (int)$size) );
  var_dump( array_chunk($input_array, (int)$size, true) );
  var_dump( array_chunk($input_array, (int)$size, false) );
}
echo "Done";
}
