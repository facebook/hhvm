<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing different arrays for $arr1 argument
 */

function callback($a)
:mixed{
  return ($a);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : different arrays for 'arr1' argument ***\n";

// different arrays
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

// loop through the various elements of $arrays to test array_map()
$iterator = 1;
foreach($arrays as $arr1) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_map(callback<>, $arr1) );
  $iterator++;
}

echo "Done";
}
