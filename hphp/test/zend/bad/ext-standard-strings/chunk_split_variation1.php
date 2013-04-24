<?php
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line %d%d
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

echo "*** Testing chunk_split() : with unexpected values for 'str' argument ***\n";

// Initialising variables
$chunklen = 2;
$ending = ' ';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//class for object variable
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

//resource  variable
$fp = fopen(__FILE__, 'r');

//different values for 'str'
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
  new MyClass(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,

  // resource data
  $fp	
);

// loop through each element of the array for 'str'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  var_dump( chunk_split($values[$count], $chunklen, $ending) );
};

echo "Done";

// close the resource
fclose($fp);

?>