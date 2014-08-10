<?php
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : with different token strings
*/

echo "*** Testing strtok() : with different token strings ***\n";
// initialize all required variables
$str = 'this testcase test strtok() function ';

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

// declaring a class
class sample  {
  public function __toString() {
  return "obj-ect";
  }
} 

// Defining resource
$file_handle = fopen(__FILE__, 'r');

// array with different values
$values =  array (

  // integer values
  0,
  1,
  12345,
  -2345,

  // float values
  10.5,
  -10.5,
  10.5e10,
  10.6E-10,
  .5,

  // array values
  array(),
  array(0),
  array(1),
  array(1, 2),
  array('color' => 'red', 'item' => 'pen'),

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // objects
  new sample(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,

  // undefined variable
  $undefined_var,

  // unset variable
  $unset_var,
 
  // resource
  $file_handle
);


// loop through each element of the array and check the working of strtok()
// when $token argument is supplied with different values

echo "\n--- Testing strtok() by supplying different values for 'token' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $token = $values [$index];

  var_dump( strtok($str, $token) );

  $counter ++;
}

// closing the resource
fclose($file_handle);

echo "Done\n";
?>
