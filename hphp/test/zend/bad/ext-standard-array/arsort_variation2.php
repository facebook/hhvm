<?php
/* Prototype  : proto bool arsort(array &array_arg [, int sort_flags])
 * Description: Sort an array and maintain index association
                Elements will be arranged from highest to lowest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * Testing arsort() by providing different unexpected values for flag argument
*/

echo "*** Testing arsort() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// resource variable
$fp = fopen(__FILE__, "r");

// temperory array for checking unexpected behavior
$unsorted_values = array(1 => 10, 2 => 2, 3 => 45);

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

// loop though each element of the array and check the working of arsort()
// when $flag argument is supplied with different values from $unexpected_values
echo "\n-- Testing arsort() by supplying different unexpected values for 'sort_flags' argument --\n";

$counter = 1;
for($index = 0; $index < count($unexpected_values); $index ++) {
  echo "-- Iteration $counter --\n";
  $value = $unexpected_values [$index];
  $temp_array = $unsorted_values;
  var_dump( arsort($temp_array, $value) ); 
  var_dump($temp_array);
  $counter++;
}

echo "Done";
?>