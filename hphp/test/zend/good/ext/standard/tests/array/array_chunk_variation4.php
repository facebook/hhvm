<?hh
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 *            : Chunks an array into size large chunks
 * Source code: ext/standard/array.c
*/

/*
* Testing array_chunk() function - input array containing different sub arrays
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : usage variations ***\n";

$size = 2;

// input array
$input_array = darray [
  "array1" => vec[],
  "array2" => vec[1, 2, 3],
  "array3" =>  vec[1]
];

echo "\n-- Testing array_chunk() by supplying an array containing different sub arrays & 'preserve_key' as defualt --\n";
var_dump( array_chunk($input_array, $size) );

echo "\n-- Testing array_chunk() by supplying an array containing different sub arrays & 'preserve_key' = true --\n";
var_dump( array_chunk($input_array, $size, true) );

echo "\n-- Testing array_chunk() by supplying an array containing different sub arrays & 'preserve_key' = false --\n";
var_dump( array_chunk($input_array, $size, false) );

echo "Done";
}
