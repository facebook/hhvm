<?hh
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/

/*
 * test join() by giving different pieces values
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing join() : usage variations ***\n";

$pieces_arrays = varray [
  vec[1, 2], // array with default keys and numrice values
  vec[1.1, 2.2], // array with default keys & float values
  vec['Array', 'Array'], // former sub arrays
  vec[false,true], // array with default keys and boolean values
  vec[], // empty array
  vec[NULL], // array with NULL
  vec["a","aaaa","b","bbbb","c","ccccc"],

  // associative arrays
  dict[1 => "one", 2 => "two", 3 => "three"],  // explicit numeric keys, string values
  dict["one" => 1, "two" => 2, "three" => 3 ],  // string keys & numeric values
  dict[ 1 => 10, 2 => 20, 4 => 40, 3 => 30],  // explicit numeric keys and numeric values
  dict[ "one" => "ten", "two" => "twenty", "three" => "thirty"],  // string key/value
  dict["one" => 1, 2 => "two", 4 => "four"],  //mixed

  // associative array, containing null/empty/boolean values as key/value
  dict['' => "NULL", '' => "null", "NULL" => NULL, "null" => null],
  dict[1 => "true", 0 => "false", "false" => false, "true" => true],
  dict["" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''],
  dict[1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true],
  dict['' => 1, "" => 2, '' => 3, '' => 4, 0 => 5, 1 => 6],

  // array with repetative keys
  dict["One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3]
];

// a multichar glue value
$glue = "], [";

// loop through each $pieces_arrays element and call join()
$iteration = 1;
for($index = 0; $index < count($pieces_arrays); $index ++) {
  echo "-- Iteration $iteration --\n";
  var_dump( join($glue, $pieces_arrays[$index]) );
  $iteration ++;
}

echo "Done\n";
}
