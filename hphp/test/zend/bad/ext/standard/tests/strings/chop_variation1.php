<?php
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : with different unexpected values for $str argument passed to the function
*/

echo "*** Testing chop() : with unexpected values for str argument ***\n";
// initialize all required variables

$charlist = " @#$%1234567890";
// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

// declaring class
class sample  {
  public function __toString() {
    return " @#$%Object @#$%";
  }
}
$sample_obj = new sample;

// creating a file resource
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
 
  // object
  $sample_obj,
  
  // resource
  $file_handle
);


// loop through each element of the array and check the working of chop()
// when $str argument is supplied with different values

echo "\n--- Testing chop() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  var_dump( chop($str) );
  var_dump( chop($str, $charlist) );

  $counter ++;
}

// closing the resource
fclose( $file_handle);

echo "Done\n";
?>