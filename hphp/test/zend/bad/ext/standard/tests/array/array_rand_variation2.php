<?php
/* Prototype  : mixed array_rand(array input [, int num_req])
 * Description: Return key/keys for random entry/entries in the array 
 * Source code: ext/standard/array.c
*/

/*
* Test array_rand() with different types of values other than int passed to 'num_req' argument
* to see that function works with unexpeced data and generates warning message as required.
*/

echo "*** Testing array_rand() : unexpected values for 'num_req' parameter ***\n";

// Initialise function arguments
$input = array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//define a class
class test
{
  var $t = 10;
  function __toString()
  {
    return "3object";
  }
}

//array of values to iterate over
$values = array(

        // int data
/*1*/   0,
        1,
        12345,
        -2345,

        // float data
/*5*/   10.5,
        -10.5,
        12.3456789000e10,
        12.3456789000E-10,
        .5,

        // null data
/*10*/  NULL,
        null,

        // boolean data
/*12*/  true,
        false,
        TRUE,
        FALSE,

        // empty data
/*16*/  "",
        '',

        // string data
/*18*/  "string",
        'string',

        // object data
/*20*/  new test(),

        // undefined data
/*21*/  @$undefined_var,

        // unset data
/*22*/  @$unset_var,
);


// loop through each element of the array for different values for 'num_req' argument
$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  var_dump( array_rand($input,$value) );  
  $count++;
};

echo "Done";
?>