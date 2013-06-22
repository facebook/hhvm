<?php
/* Prototype  : string ucwords ( string $str )
 * Description: Uppercase the first character of each word in a string
 * Source code: ext/standard/string.c
*/

/*
 * Test ucwords() by passing different values including scalar and non scalar values 
*/

echo "*** Testing ucwords() : usage variations ***\n";
// initialize all required variables

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

$fp = fopen(__FILE__, "r");

class my
{
  function __toString() {
    return "myString";
  }
}

// array with different values
$values =  array (

  // integer values
  0,
  1,
  12345,
  -2345,

  // hex values 
  0x10,
  0X20,
  0xAA,
  -0XF5,

  // octal values
  0123,
  -0342,

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
  new my(),

  // empty string
  "",
  '',

  //NULL
  NULL,
  null,

  // hex in string 
  "0x123",
  '0x123',
  "0xFF12",
  "-0xFF12",
  
  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var,

  // resource variable
  $fp
);

// loop through each element of the array and check the working of ucwords()
// when $str argument is supplied with different values
echo "\n--- Testing ucwords() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  var_dump( ucwords($str) );

  $counter ++;
}

// close the file handle
fclose($fp);
echo "Done\n";
?>