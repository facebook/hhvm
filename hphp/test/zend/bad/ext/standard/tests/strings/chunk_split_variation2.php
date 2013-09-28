<?php
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

echo "*** Testing chunk_split() : with unexpected values for 'chunklen' argument ***\n";

// Initialise function arguments
$str = 'This is chuklen variation';
$ending = '*';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//get resource variable
$fp = fopen(__FILE__, 'r');

//Class to get object variable
class MyClass
{
   public function __toString() {
     return "object";
   }
}

//array of values to iterate over
$values = array(

  // float data
  10.5,
  -10.5,
  (float) PHP_INT_MAX + 1,
  (float) -PHP_INT_MAX - 1,
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

  // resource variable
  $fp
);

// loop through each element of the values for 'chunklen'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  var_dump( chunk_split($str, $values[$count], $ending) );
}

//closing resource
fclose($fp);

?>
===DONE===