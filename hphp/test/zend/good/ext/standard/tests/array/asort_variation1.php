<?php
/* Prototype  : bool asort(array &array_arg [, int sort_flags])
 * Description: Sort an array and maintain index association
                Elements will be arranged from lowest to highest when this function has completed. 
 * Source code: ext/standard/array.c
*/

/*
 * testing asort() by providing different unexpected values for array argument with following flag values.
 * 1. flag value as defualt
 * 2. SORT_REGULAR - compare items normally
 * 3. SORT_NUMERIC - compare items numerically
 * 4. SORT_STRING - compare items as strings
*/

echo "*** Testing asort() : usage variations ***\n";

// get an unset variable
$unset_var = 10;
unset ($unset_var);

// resource variable
$fp = fopen(__FILE__, "r");

//array of values with indices to iterate over
$unexpected_values = array (

  // int data
  0 => 0,
  1 => 1,
  2 => 12345,
  3 => -2345,

  // float data
  4 => 10.5,
  5 => -10.5,
  6 => 10.5e3,
  7 => 10.6E-2,
  8 => .5,

  // null data
  9 => NULL,
  10 => null,

  // boolean data
  11 => true,
  12 => false,
  13 => TRUE,
  14 => FALSE,

  // empty data
  15 => "",
  16 => '',

  // string data
  17 => "string",
  18 => 'string',

  // object data
  19 => new stdclass(),

  // undefined data
  20 => @undefined_var,

  // unset data
  21 => @unset_var,

  // resource variable
  22 => $fp

);

// loop though each element of the array and check the working of asort()
// when $array argument is supplied with different values from $unexpected_values
echo "\n-- Testing asort() by supplying different unexpected values for 'array' argument --\n";
echo "\n-- Flag values are defualt, SORT_REGULAR, SORT_NUMERIC, SORT_STRING --\n"; 

$counter = 1;
for($index = 0; $index < count($unexpected_values); $index ++) {
  echo "-- Iteration $counter --\n";
  $value = $unexpected_values [$index];
  var_dump( asort($value) ); // expecting : bool(false)
  var_dump( asort($value, SORT_REGULAR) ); // expecting : bool(false)
  var_dump( asort($value, SORT_NUMERIC) ); // expecting : bool(false)
  var_dump( asort($value, SORT_STRING) ); // expecting : bool(false)
  $counter++;
}

echo "Done";
?>