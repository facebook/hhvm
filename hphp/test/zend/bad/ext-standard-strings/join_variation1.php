<?php
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/

/*
 * testing join() by passing different unexpected value for glue argument
*/

echo "*** Testing join() : usage variations ***\n";
// initialize all required variables
$pieces = array("element1", "element2");

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

// get a resource variable
$fp = fopen(__FILE__, "r");

// define a class
class test
{
   var $t = 10;
   function __toString() {
     return  "testObject";
   }
}

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
  new test(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,
  
  // resource variable
  $fp,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);


// loop through each element of the array and check the working of join()
// when $glue argument is supplied with different values
echo "\n--- Testing join() by supplying different values for 'glue' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $glue = $values [$index];

  var_dump( join($glue, $pieces) );

  $counter ++;
}

echo "Done\n";
?>