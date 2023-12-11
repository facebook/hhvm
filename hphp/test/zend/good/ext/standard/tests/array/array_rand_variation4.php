<?hh
/* Prototype  : mixed array_rand(array $input [, int $num_req])
 * Description: Return key/keys for random entry/entries in the array
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of array_rand() function when associative array is passed to
* the 'input' parameter in the function call
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_rand() : with associative arrays ***\n";

// initialise associative arrays
$asso_arrays = vec[

       // array with numeric keys
/*1*/  dict[1 => 'one', 2 => 2, 1234567890 => 'big', -1 => 'negative key',
             2 => 'float key', 0 => "zero key", 0 => 'decimal key',
             (int)2e2 => 'exp key1', (int)-2e3 => 'negative exp key'],

       // array with string keys
       dict['one' => 1, "two" => 2.0, "three" => 'three',
             '12twelve' => 12.00, "" => 'empty string', " " => "space key"],

       // array with hexa values as keys
/*3*/  dict[0xabc => 2748, 0x12f => '303', 0xff => "255", -0xff => "-255"],

       // array with octal values as keys
       dict[0123 => 83, 012 => 10, 010 => "8", -034 => "-28", 0012 => '10'],

       // array with bool values as keys
       dict[1 => '1', 1 => true, 1 => "TRUE",
             0 => '0', 0 => false, 0 => "FALSE"],

       // array with special chars as keys
/*6*/  dict['##' => "key1", '&$r' => 'key2', '!' => "key3", '<>' =>'key4',
             "NULL" => 'key5', "\n" => 'newline as key',
             "\t" => "tab as key", "'" => 'single quote as key',
             '"' => 'double quote as key', "\0" => "null char as key"]
];

/* looping to test array_rand() function with different arrays having
 * different types of keys
*/
$counter = 1;
foreach($asso_arrays as $input) {
  echo "\n-- Iteration $counter --\n";

  // with default argument
  echo"\nWith default argument\n";
  var_dump( array_rand($input) );

  // with default and optional arguments
  echo"\nWith num_req = 1\n";
  var_dump( array_rand($input, 1) );  // with $num_req=1
  echo"\nWith num_req = 2\n";
  var_dump( array_rand($input, 2) );  // with $num_req=2

  $counter++;
}  // end of for loop


echo "Done";
}
