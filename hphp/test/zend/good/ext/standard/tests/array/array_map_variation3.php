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
/*13*/ darray['' => "NULL", '' => "null", "NULL" => NULL, "null" => null],
       darray[1 => "true", 0 => "false", "false" => false, "true" => true],
       darray["" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''],
       darray[1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true],
       darray['' => 1, "" => 2, '' => 3, '' => 4, 0 => 5, 1 => 6],

       // array with repetative keys
/*18*/ darray["One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3]
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
