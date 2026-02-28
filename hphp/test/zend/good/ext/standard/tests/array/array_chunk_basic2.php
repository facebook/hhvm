<?hh
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 *              Chunks an array into size  large chunks.
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : basic functionality ***\n";
$size = 2;

$input_arrays = vec[
  // array with default keys - numeric values
  vec[1, 2, 3, 4, 5],

  // array with default keys - string values
  vec['value1', "value2", "value3"],

  // associative arrays - key as string
  dict['key1' => 1, "key2" => 2, "key3" => 3],

  // associative arrays - key as numeric
  dict[1 => 'one', 2 => "two", 3 => "three"],

  // array containing elements with/without keys
  dict[1 => 'one', 2 => 'two', 3 => 'three', 4 => 4, "five" => 5]
];

$count = 1;
// loop through each element of the array for input
foreach ($input_arrays as $input_array){
  echo "\n-- Iteration $count --\n";
  var_dump( array_chunk($input_array, $size, true) );
  var_dump( array_chunk($input_array, $size, false) );
  $count++;
}

echo "Done";
}
