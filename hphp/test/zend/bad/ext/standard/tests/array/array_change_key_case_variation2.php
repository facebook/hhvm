<?php
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased] 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $case argument to array_change_key_case() to test behaviour
 * Where possible, CASE_UPPER has been entered as a string value
 */

echo "*** Testing array_change_key_case() : usage variations ***\n";

// Initialise function arguments not being substituted
$array = array ('one' => 1, 'TWO' => 2, 'Three' => 3);

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
CASE_UPPER
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $case argument
$inputs = array(

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
       array(),

       // string data
/*19*/ "CASE_UPPER",
       'CASE_UPPER',
       $heredoc,

       // undefined data
/*22*/ @$undefined_var,

       // unset data
/*23*/ @$unset_var,
);

// loop through each element of $inputs to check the behavior of array_change_key_case()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( array_change_key_case($array, $input) );
  $iterator++;
};

echo "Done";
?>
