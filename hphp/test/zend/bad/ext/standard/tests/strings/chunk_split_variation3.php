<?php
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

echo "*** Testing chunk_split() : unexpected values for 'ending' ***\n";

// Initializing variables
$str = 'This is simple string.';
$chunklen = 4.9;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//resource variable
$fp = fopen(__FILE__,'r');

//Class to get object variable
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

//different values for 'ending'
$values = array(

  // int data
  0,
  1,
  12345,
  -2345,

  // float data
  10.5,
  -10.5,
  10.123456e10,
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

  // object data
  new MyClass(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,
	
  // resource data
  $fp
);

// loop through each element of values for 'ending'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  var_dump( chunk_split($str, $chunklen, $values[$count]) );
}

echo "Done";

//closing resource
fclose($fp);
?>