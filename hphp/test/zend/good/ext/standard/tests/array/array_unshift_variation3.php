<?hh
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the behavior of array_unshift() by passing different types of arrays
 * to $array argument to which the $var arguments will be prepended
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unshift() : different arrays for \$array argument ***\n";

// initialize $var argument
$var = 10;

// different arrays to be passed to $array argument
$arrays = vec[
/*1*/  vec[1, 2], // array with default keys and numeric values
       vec[1.1, 2.2], // array with default keys & float values
       vec[ vec[2], vec[1]], // sub arrays
       vec[false,true], // array with default keys and boolean values
       vec[], // empty array
       vec[NULL], // array with NULL
       vec["a","aaaa","b","bbbb","c","ccccc"],

       // associative arrays
/*8*/  dict[1 => "one", 2 => "two", 3 => "three"],  // explicit numeric keys, string values
       dict["one" => 1, "two" => 2, "three" => 3 ],  // string keys & numeric values
       dict[ 1 => 10, 2 => 20, 4 => 40, 3 => 30],  // explicit numeric keys and numeric values
       dict[ "one" => "ten", "two" => "twenty", "three" => "thirty"],  // string key/value
       dict["one" => 1, 2 => "two", 4 => "four"],  //mixed

       // associative array, containing null/empty/boolean values as key/value
/*13*/ dict['' => "NULL", '' => "null", "NULL" => NULL, "null" => null],
       dict[1 => "true", 0 => "false", "false" => false, "true" => true],
       dict["" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''],
       dict[1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true],
       dict['' => 1, "" => 2, '' => 3, '' => 4, 0 => 5, 1 => 6],

       // array with repetative keys
/*18*/ dict["One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3]
];

// loop through the various elements of $arrays to test array_unshift()
$iterator = 1;
foreach($arrays as $array) {
  echo "-- Iteration $iterator --\n";

  /* with default argument */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  $temp_array = $array;
  var_dump( array_unshift(inout $temp_array, $var) );

  // dump the resulting array
  var_dump($temp_array);

  /* with optional arguments */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  $temp_array = $array;
  var_dump( array_unshift(inout $temp_array, $var, "hello", 'world') );

  // dump the resulting array
  var_dump($temp_array);
  $iterator++;
}

echo "Done";
}
