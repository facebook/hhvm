<?php
/* Prototype  : bool sort(array &array_arg [, int $sort_flags])
 * Description: Sort an array 
 * Source code: ext/standard/array.c
*/

/*
 * testing sort() by providing different unexpected values for array argument
 * with following flag values.
 * 1. flag value as defualt
 * 2. SORT_REGULAR - compare items normally
 * 3. SORT_NUMERIC - compare items numerically
 * 4. SORT_STRING - compare items as strings
*/

echo "*** Testing sort() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// resource variable
$fp = fopen(__FILE__, "r");

//array of values to iterate over
$unexpected_values = array (

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       10.5e3,
       10.6E-2,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "string",
       'string',

       // object data
/*20*/ new stdclass(),

       // undefined data
/*21*/ @undefined_var,

       // unset data
/*22*/ @unset_var,

       // resource variable
/*23*/ $fp
);

// loop though each element of the array and check the working of sort()
// when $array argument is supplied with different values from $unexpected_values 
echo "\n-- Testing sort() by supplying different unexpected values for 'array' argument --\n";
echo "\n-- Flag values are defualt, SORT_REGULAR, SORT_NUMERIC, SORT_STRING --\n"; 

$counter = 1;
for($index = 0; $index < count($unexpected_values); $index ++) {
  echo "-- Iteration $counter --\n";
  $value = $unexpected_values [$index];
  var_dump( sort($value) ); // expecting : bool(false)
  var_dump( sort($value, SORT_REGULAR) ); // expecting : bool(false)
  var_dump( sort($value, SORT_NUMERIC) ); // expecting : bool(false)
  var_dump( sort($value, SORT_STRING) ); // expecting : bool(false)
  $counter++;
}

echo "Done";
?>