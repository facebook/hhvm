<?php
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function with unexpected inputs for haystack 
 *  and expected type for 'needle' 
*/

echo "*** Testing strrchr() function: with unexpected inputs for haystack ***\n";

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

// declaring a class
class sample  {
  public function __toString() {
    return "object";
  } 
}

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values
$haystacks =  array (

  // integer values
  0,
  1,
  12345,
  -2345,

  // float values
  10.5,
  -10.5,
  10.5e10,
  10.6E-10,
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

  // null vlaues
  NULL,
  null,

  // objects
  new sample(),

  // empty string
  "",
  '',

  // resource
  $file_handle,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);

$needles =  array (
  //integer numeric strings
  "0",
  "1",
  "2",
  "-2",

  //float numeric strings
  "10.5",
  "-10.5",
  "10.5e10",
  "10.6E-10",
  ".5",

  //regular strings
  "array",
  "a",
  "r",
  "y",
  "ay",
  "true",
  "false",
  "TRUE",
  "FALSE",
  "NULL",
  "null",
  "object",

  //empty string
  "",
  '',
 
  //resource variable in string form
  "\$file_handle",

  //undefined variable in string form
  @"$undefined_var",
  @"$unset_var"
);

// loop through each element of the array and check the working of strrchr()
$count = 1;
for($index = 0; $index < count($haystacks); $index++) {
  echo "-- Iteration $count --\n";
  var_dump( strrchr($haystacks[$index], $needles[$index]) );
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***";
?>