<?php
/* Prototype  : string htmlspecialchars_decode(string $string [, int $quote_style])
 * Description: Convert special HTML entities back to characters 
 * Source code: ext/standard/html.c
*/

/*
 * testing htmlspecialchars_decode() by giving unexpected input values for $quote_style argument
*/

echo "*** Testing htmlspecialchars_decode() : usage variations ***\n";

// Initialise function arguments
// value initialized = Roy's height > Sam's height. 13 < 15. 1111 & 0000 = 0000. " double quote string " 
$string = "<html>Roy&#039;s height &gt; Sam&#039;s height. 13 &lt; 15. 1111 &amp; 0000 = 0000. &quot; double quote string &quot;</html>";

//get a class
class classA {
  function __toString() {
    return "Class A Object";
  }
}

//get a resource variable
$file_handle = fopen(__FILE__, "r");

//get an unset variable
$unset_var = 10;
unset($unset_var);

//array of values to iterate over
$values = array(

      // float data
      10.5,
      -10.5,
      10.5e20,
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

      // string data
      "string",
      'string',

      // object data
      new classA(),

      // undefined data
      @$undefined_var,

      // unset data
      @$unset_var,

      //resource
      $file_handle
);

// loop through each element of the array for quote_style
$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( htmlspecialchars_decode($string, $value) );
      $iterator++;
}

// close the file resource used
fclose($file_handle);

echo "Done";
?>