<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

error_reporting(E_ALL & ~E_NOTICE);

echo "*** Testing sprintf() : with different types of values passed for arg1 argument ***\n";

// initialing required variables
$format = '%s';
$arg2 = 'third argument';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// declaring class
class sample
{
  public function __toString() {
    return "Object";
  } 
}

// creating a file resource
$file_handle = fopen(__FILE__, 'r');

//array of values to iterate over
$values = array(

      // int data
      0,
      1,
      12345,
      -2345,

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      array(),
      array(0),
      array(1),
      array(1, 2),
      array('color' => 'red', 'item' => 'pen'),

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // string data
      "string",
      'string',

      // object data
      new sample(),

      // undefined data
      @$undefined_var,

      // unset data
      @$unset_var,

      // resource data
      $file_handle
);

// loop through each element of the array for arg1

$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  
  // with two arguments
  var_dump( sprintf($format, $value) );

  // with three arguments
  var_dump( sprintf($format, $value, $arg2) );
  
  $count++;   
};

// closing the resource
fclose($file_handle);

echo "Done";
?>