<?php
/* Prototype  : string nl2br(string $str)
 * Description: Inserts HTML line breaks before all newlines in a string.
 * Source code: ext/standard/string.c
*/

/*
* Test nl2br() function by passing different types of values other than 
*   expected type for 'str' argument
*/

echo "*** Testing nl2br() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//getting resource
$file_handle = fopen(__FILE__, "r");

//defining class
class Sample {
  public function __toString() {
    return "My String";
  }
}

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

  //resource
  $file_handle,

  // object data
  new Sample(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,
);

// loop through $values array to test nl2br() function with each element 
$count = 1;
foreach($values as $value) {
  echo "-- Iteration $count --\n";
  var_dump( nl2br($value) );
  $count ++ ;
};

//closing the file handle
fclose( $file_handle );

echo "Done";
?>