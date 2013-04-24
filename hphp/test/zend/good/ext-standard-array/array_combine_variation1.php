<?php
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys 
 *              and the elements of the second as the corresponding values 
 * Source code: ext/standard/array.c
*/

/*
* Testing array_combine() function by passing values to $keys argument other than arrays
* and see that function emits proper warning messages wherever expected.
* The $values argument passed is a fixed array.
*/

echo "*** Testing array_combine() : Passing non-array values to \$keys argument ***\n";

// Initialise $values argument 
$values = array(1, 2);

//get an unset variable
$unset_var = 10;
unset($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "Class A object";
  }
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $keys argument
$keys_passed = array(

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
       $heredoc,

       // object data
/*21*/ new classA(),

       // undefined data
/*22*/ @$undefined_var,

       // unset data
/*23*/ @$unset_var,

       // resource variable
/*24*/ $fp
);

// loop through each element within $keys_passed to check the behavior of array_combine()
$iterator = 1;
foreach($keys_passed as $keys) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_combine($keys,$values) );
  $iterator++;
};

echo "Done";
?>