<?php
/* Prototype  : bool krsort(array &array_arg [, int sort_flags])
 * Description: Sort an array by key in reverse order, maintaining key to data correlation 
 * Source code: ext/standard/array.c
*/

/*
 * Testing krsort() by providing different unexpected values for flag argument
*/

echo "*** Testing krsort() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// resource variable
$fp = fopen(__FILE__, "r");

// an array for checking unexpected behavior
$unsorted_values = array(10 => 10, 2 => 2, 45 => 45);

//array of unexpected values to iterate over
$unexpected_values = array (

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

// loop though each element of the array and check the working of krsort()
// when 'sort_flags' argument is supplied with different values
echo "\n-- Testing krsort() by supplying different unexpected values for 'sort_flags' argument --\n";

$counter = 1;
for($index = 0; $index < count($unexpected_values); $index ++) {
  echo "-- Iteration $counter --\n";
  $value = $unexpected_values [$index];
  $temp_array = $unsorted_values;
  var_dump( krsort($temp_array, $value) ); 
  var_dump($temp_array);
  $counter++;
}

echo "Done";
?>
