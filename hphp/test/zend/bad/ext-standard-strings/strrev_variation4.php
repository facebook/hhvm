<?php
/* Prototype  : string strrev(string $str);
 * Description: Reverse a string 
 * Source code: ext/standard/string.c
*/

/* Testing strrev() function with unexpected inputs for 'str' */

echo "*** Testing strrev() : unexpected inputs for 'str' ***\n";
//class declaration
class sample {
  public function __toString(){
    return "object";
  }
}

//get the resource 
$resource = fopen(__FILE__, "r");

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = array(

  // int data
  0,
  1,
  12345,
  -2345,

  // float data
  10.5,
   -10.5,
  10.5e10,
  10.6E-10,
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
  new sample(),

  // resource
  $resource,

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var
);

// loop through each element of the array for str

$count = 1;
foreach($values as $value) {
  echo "\n-- Iterator $count --\n";
  var_dump( strrev($value) );
  $count++;
};

fclose($resource);  //closing the file handle

echo "*** Done ***";
?>