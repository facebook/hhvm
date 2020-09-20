<?hh
/* Prototype  : mixed array_rand(array $input [, int $num_req])
 * Description: Return key/keys for random entry/entries in the array
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of array_rand() function when multi-dimensional array
* is passed to 'input' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_rand() : with multi-dimensional array ***\n";

// initialise the multi-dimensional array
$input = varray[
       // array with int values
/*1*/  varray[1, 2, 0, -0, -1, -2],

       // array with float values
       varray[1.23, -1.23, 0.34, -0.34, 0e2, 2e-3, -2e2, -40e-2],

       // array with single quoted strings
/*3*/  varray['one', '123numbers', 'hello\tworld', 'hello world\0', '12.34floatnum'],

       // array with double quoted strings
       varray["one","123numbers", "hello\tworld", "hello world\0", "12.34floatnum"],

       // array with bool values
/*5*/  varray[true, TRUE, FALSE, false, TrUe, FaLsE],

       // array with hexa values
       varray[0x123, -0x123, 0xabc, 0xABC, 0xab],

       // array with null values
/*7*/  varray[null, NULL, "\0", Null, NuLl]

];

// initialise 'num_req' variable
$num_req = 3;

// calling array_rand() function with multi-dimensional array
var_dump( array_rand($input, $num_req) );

// looping to test array_rand() with each sub-array in the multi-dimensional array
echo "\n*** Testing array_rand() with arrays having different types of values ***\n";
$counter = 1;
foreach($input as $arr) {
  echo "\n-- Iteration $counter --\n";
  var_dump( array_rand($arr) );  // with default arguments
  var_dump( array_rand($arr, 3) );  // with default as well as optional arguments
  $counter++;
}

echo "Done";
}
