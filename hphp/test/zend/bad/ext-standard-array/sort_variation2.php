<?php
/* Prototype  : bool sort(array &array_arg [, int $sort_flags])
 * Description: Sort an array 
 * Source code: ext/standard/array.c
*/

/*
 * Testing sort() by providing different unexpected values for flag argument
*/

echo "*** Testing sort() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// resource variable
$fp = fopen(__FILE__, "r");

// temperory array for checking unexpected behavior
$unsorted_values = array(10, 2, 45);

//array of values to iterate over
$unexpected_values = array(

       // int data
/*1*/  -2345,

       // float data
/*2*/  10.5,
       -10.5,
       10.5e2,
       10.6E-2,
       .5,

       // null data
/*7*/  NULL,
       null,

       // boolean data
/*9*/  true,
       false,
       TRUE,
       FALSE,

       // empty data
/*13*/ "",
       '',

       // string data
/*15*/ "string",
       'string',

       // object data
/*16*/ new stdclass(),

       // undefined data
/*17*/ @undefined_var,

       // unset data
/*18*/ @unset_var,

       // resource variable
/*19*/ $fp

);

// loop though each element of the array and check the working of sort()
// when $flag arugment is supplied with different values
echo "\n-- Testing sort() by supplying different unexpected values for 'flag' argument --\n";

$counter = 1;
for($index = 0; $index < count($unexpected_values); $index ++) {
  echo "-- Iteration $counter --\n";

  // sort the array, retain a temp. copy of input array for next iteration
  $value = $unexpected_values [$index];
  $temp_array = $unsorted_values;
  var_dump( sort($temp_array, $value) ); 
  
  //dump the sorted array
  var_dump($temp_array);
  $counter++;
}

echo "Done";
?>