<?php
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/

/*
 * test wordwrap by passing different values for width argument 
*/
echo "*** Testing wordwrap() : usage variations ***\n";
// initialize all required variables
$str = 'testing wordwrap function';
$break = '<br />\n';
$cut = true;

// resource var 
$fp = fopen(__FILE__, "r");

// get an unset variable
$unset_var = 10;
unset($unset_var);


// array with different values as width
$values =  array (
  // zerovalue for width
  0,

  // -ve value for width
  -1,
  -10,

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

  // string values
  "string",
  'string',

  // objects
  new stdclass(),

  // Null value 
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
// when $width argument is supplied with different values
echo "\n--- Testing wordwrap() by supplying different values for 'width' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $width = $values [$index];

  var_dump( wordwrap($str, $width) );
  var_dump( wordwrap($str, $width, $break) );

  // cut as false 
  $cut = false;
  var_dump( wordwrap($str, $width, $break, $cut) );

  // cut as true
  $cut = true;
  var_dump( wordwrap($str, $width, $break, $cut) );

  $counter ++;
}

// close the resource
fclose($fp);

echo "Done\n";
?>