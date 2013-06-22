<?php
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/

/*
 * testing wordwrap() by providing different values for str argument
*/

echo "*** Testing wordwrap() : usage variations ***\n";
// initialize all required variables
$width = 3;
$break = '<br />\n';
$cut = true;

// resource variable
$fp = fopen(__FILE__, "r");

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

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
  new stdclass(),

  // Null
  NULL,
  null,

  // empty string
  "",
  '',

  // resource variable
  $fp,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);

// loop though each element of the array and check the working of wordwrap()
// when $str argument is supplied with different values
echo "\n--- Testing wordwrap() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  var_dump( wordwrap($str) );
  var_dump( wordwrap($str, $width) );
  var_dump( wordwrap($str, $width, $break) );

  // $cut as false
  $cut = false;
  var_dump( wordwrap($str, $width, $break, $cut) );

  // $cut as true
  $cut = true;
  var_dump( wordwrap($str, $width, $break, $cut) );

  $counter ++;
}

// close the resource 
fclose($fp);

echo "Done\n";
?>