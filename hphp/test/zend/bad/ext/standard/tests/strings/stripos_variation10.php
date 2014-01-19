<?php
/* Prototype  : int stripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of first occurrence of a case-insensitive string
 * Source code: ext/standard/string.c
*/

/* Test stripos() function with unexpected inputs for 'needle' and 
 *  an expected type of input for 'haystack' argument 
*/

echo "*** Testing stripos() function with unexpected values for needle ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//defining a class
class sample  {
  public function __toString() {
    return "object";
  } 
}

//getting the resource
$file_handle = fopen(__FILE__, "r");

$haystack = "string 0 1 2 -2 10.5 -10.5 10.5e10 10.6E-10 .5 array true false object \"\" null Resource";

// array with different values
$needles =  array (

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
  new sample(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,

  // resource
  $file_handle,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);

// loop through each element of the 'needle' array to check the working of stripos()
$counter = 1;
for($index = 0; $index < count($needles); $index ++) {
  echo "\n-- Iteration $counter --\n";
  var_dump( stripos($haystack, $needles[$index]) );
  $counter ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***";
?>