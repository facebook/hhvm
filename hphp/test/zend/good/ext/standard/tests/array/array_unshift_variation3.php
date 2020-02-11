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
$arrays = varray [
/*1*/  varray[1, 2], // array with default keys and numeric values
       varray[1.1, 2.2], // array with default keys & float values
       varray[ varray[2], varray[1]], // sub arrays
       varray[false,true], // array with default keys and boolean values
       varray[], // empty array
       varray[NULL], // array with NULL
       varray["a","aaaa","b","bbbb","c","ccccc"],

       // associative arrays
/*8*/  darray[1 => "one", 2 => "two", 3 => "three"],  // explicit numeric keys, string values
       darray["one" => 1, "two" => 2, "three" => 3 ],  // string keys & numeric values
       darray[ 1 => 10, 2 => 20, 4 => 40, 3 => 30],  // explicit numeric keys and numeric values
       darray[ "one" => "ten", "two" => "twenty", "three" => "thirty"],  // string key/value
       darray["one" => 1, 2 => "two", 4 => "four"],  //mixed

       // associative array, containing null/empty/boolean values as key/value
/*13*/ darray[NULL => "NULL", null => "null", "NULL" => NULL, "null" => null],
       darray[true => "true", false => "false", "false" => false, "true" => true],
       darray["" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''],
       darray[1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true],
       darray['' => 1, "" => 2, NULL => 3, null => 4, false => 5, true => 6],

       // array with repetative keys
/*18*/ darray["One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3]
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
