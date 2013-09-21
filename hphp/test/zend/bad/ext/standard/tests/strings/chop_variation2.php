<?php
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : with different unexpected values for charlist argument passes to the function
*/

echo "*** Testing chop() : with different unexpected values for charlist argument ***\n";
// initialize all required variables
$str = 'hello world12345 ';

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

// declaring class
class sample  {
  public function __toString()  {
  return "@# $%12345";
  }
}

// defining a resource
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
  10.1234567e10,
  10.7654321E-10,
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

  // null values
  NULL,
  null,

  // resource
  $file_handle,
  
  // undefined variable
  $undefined_var,

  // unset variable
  $unset_var
  
);


// loop through each element of the array and check the working of chop()
// when $charlist argument is supplied with different values

echo "\n--- Testing chop() by supplying different values for 'charlist' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $charlist = $values [$index];

  var_dump( chop($str, $charlist) );

  $counter ++;
}

// closing the resource
fclose($file_handle);

echo "Done\n";
?>