<?php

/* Prototype  : array curl_version  ([ int $age  ] )
 * Description: Returns information about the cURL version.
 * Source code: ext/curl/interface.c
*/

echo "*** Testing curl_version() function: with unexpected inputs for 'age' argument ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//defining a class
class sample  {
  public function __toString() {
    return "sample object";
  } 
}

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  array (

  // integer values
  0,
  1,
  255,
  256,
  PHP_INT_MAX,
  -PHP_INT_MAX,

  // float values
  10.5,
  -20.5,
  10.1234567e10,

  // array values
  array(),
  array(0),
  array(1, 2),
  
  //string values
  "ABC",
  'abc',
  "2abc",

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // null values
  NULL,
  null,

  // objects
  new sample(),

  // resource
  $file_handle,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);

// loop through with each element of the $inputs array to test curl_version() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  var_dump( is_array(curl_version($input)) );
  $count ++;
}

fclose($file_handle);  //closing the file handle

?>
===Done===