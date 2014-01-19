<?php
/* Prototype  : proto string utf8_encode(string data)
 * Description: Encodes an ISO-8859-1 string to UTF-8 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing utf8_encode() : usage variations ***\n";
error_reporting(E_ALL & ~E_NOTICE);

class aClass {
   function __toString() {
       return "Some Ascii Data";
   }
}

// Initialise function arguments not being substituted (if any)

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

      // object data
      new aClass(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for data

foreach($values as $value) {
      echo @"\nArg value $value \n";
      var_dump( utf8_encode($value) );
};

echo "Done";
?>