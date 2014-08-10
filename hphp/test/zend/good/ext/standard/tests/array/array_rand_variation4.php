<?php
/* Prototype  : mixed array_rand(array $input [, int $num_req])
 * Description: Return key/keys for random entry/entries in the array 
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of array_rand() function when associative array is passed to
* the 'input' parameter in the function call
*/

echo "*** Testing array_rand() : with associative arrays ***\n";

// initialise associative arrays
$asso_arrays = array(
    
       // array with numeric keys
/*1*/  array(1 => 'one', 2 => 2, 1234567890 => 'big', -1 => 'negative key',
             2.3 => 'float key', 0 => "zero key", 0.2 => 'decimal key',
             2e2 => 'exp key1', -2e3 => 'negative exp key'),
      
       // array with string keys
       array('one' => 1, "two" => 2.0, "three" => 'three',
             '12twelve' => 12.00, "" => 'empty string', " " => "space key"),

       // array with hexa values as keys
/*3*/  array(0xabc => 2748, 0x12f => '303', 0xff => "255", -0xff => "-255"),

       // array with octal values as keys 
       array(0123 => 83, 0129 => 10, 010 => "8", -0348 => "-28", 0012 => '10'),

       // array with bool values as keys
       array(TRUE => '1', true => true, TrUe => "TRUE",
             FALSE => '0', false => false, FaLsE => "FALSE"),

       // array with special chars as keys
/*6*/  array('##' => "key1", '&$r' => 'key2', '!' => "key3", '<>' =>'key4',
             "NULL" => 'key5', "\n" => 'newline as key',
             "\t" => "tab as key", "'" => 'single quote as key',
             '"' => 'double quote as key', "\0" => "null char as key")
);
       
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
?>
