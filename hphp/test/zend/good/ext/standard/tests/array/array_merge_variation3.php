<?php
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays of different data types to test how array_merge adds them
 * onto an existing array
 */

echo "*** Testing array_merge() : usage variations ***\n";

// Initialise function arguments not being substituted
$arr = array (1, 2);

//get an unset variable
$unset_var = 10;
unset ($unset_var);

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

// arrays of different data types to be passed as $input
$inputs = array(

       // int data
/*1*/  'int' => array(
	   0,
       1,
       12345,
       -2345,
       ),

       // float data
/*2*/  'float' => array(
       10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,
       ),

       // null data
/*3*/ 'null' => array(
       NULL,
       null,
       ),

       // boolean data
/*4*/ 'bool' => array(
       true,
       false,
       TRUE,
       FALSE,
       ),
       
       // empty data
/*5*/ 'empty string' => array(
       "",
       '',
       ),
       
/*6*/ 'empty array' => array(
       ),

       // string data
/*7*/ 'string' => array(
       "string",
       'string',
       $heredoc,
       ),
       
       // object data
/*8*/ 'object' => array(
       new classA(),
       ),

       // undefined data
/*9*/ 'undefined' => array(
       @$undefined_var,
       ),

       // unset data
/*10*/ 'unset' => array(
       @$unset_var,
       ),

       // resource variable
/*11*/ 'resource' => array(
       $fp
       ),
);

// loop through each element of $inputs to check the behavior of array_merge
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator: $key data --\n";
  var_dump( array_merge($input, $arr) );
  var_dump( array_merge($arr, $input) );
  $iterator++;
};

fclose($fp);

echo "Done";
?>
