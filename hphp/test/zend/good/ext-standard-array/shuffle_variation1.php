<?php
/* Prototype  : bool shuffle(array $array_arg)
 * Description: Randomly shuffle the contents of an array 
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of shuffle() when unexpected values are passed for 'array_arg'
* argument and verify that function outputs required warning messages wherever applicable
*/

echo "*** Testing shuffle() : with unexpected values for 'array_arg' argument ***\n";


//get an unset variable
$unset_var = 10;
unset ($unset_var);

//get a resource variable
$fp = fopen(__FILE__, "r");

//define a class
class test
{
  var $t = 10;
  function __toString()
  {
    return "object";
  }
}

//array of values to iterate over
$values = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
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
/*20*/ new test(),

       // undefined data
/*21*/ @$undefined_var,

       // unset data
/*22*/ @$unset_var,

/*23*/ // resource data
       $fp
);

// loop through the array to test shuffle() function
// with each element of the array
$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  var_dump( shuffle($value) );
  $count++;
};

// closing the resource
fclose($fp);

echo "Done";
?>