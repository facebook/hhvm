<?php
/* Prototype  : int array_push(array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array 
 * Source code: ext/standard/array.c
 */

/*
 * Pass array_push arrays where the keys are different data types.
 */

echo "*** Testing array_push() : usage variations ***\n";

// Initialise function arguments not being substituted
$var = 'value';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays of different data types as keys to be passed to $stack argument
$inputs = array(

       // int data
/*1*/  'int' => array(
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ),

       // float data
/*2*/  'float' => array(
       10.5 => 'positive',
       -10.5 => 'negative',
       .5 => 'half',
       ),
       
       'extreme floats' => array(
       12.3456789000e10 => 'large',
       12.3456789000E-10 => 'small',
       ),

       // null data
/*3*/ 'null uppercase' => array(
       NULL => 'null 1',
       ), 
       'null lowercase' => array(
       null => 'null 2',
       ),

       // boolean data
/*4*/ 'bool lowercase' => array(
       true => 'lowert',
       false => 'lowerf',
       ),
       'bool uppercase' => array(
       TRUE => 'uppert',
       FALSE => 'upperf',
       ),
       
       // empty data
/*5*/ 'empty double quotes' => array(
       "" => 'emptyd',
       ),
       'empty single quotes' => array(
       '' => 'emptys',
       ),

       // string data
/*6*/ 'string' => array(
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ),

       // undefined data
/*8*/ 'undefined' => array(
       @$undefined_var => 'undefined',
       ),

       // unset data
/*9*/ 'unset' => array(
       @$unset_var => 'unset',
       ),
);

// loop through each sub-array of $inputs to check the behavior of array_push()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator : $key data --\n";
  echo "Before : ";
  var_dump(count($input));
  echo "After  : ";
  var_dump( array_push($input, $var) );
  $iterator++;
};

echo "Done";
?>
