<?php
/* Prototype  : proto int xml_set_element_handler(resource parser, string shdl, string ehdl)
 * Description: Set up start and end element handlers 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_set_element_handler() : usage variations ***\n";
error_reporting(E_ALL & ~E_NOTICE);

class aClass {
   function __toString() {
       return "Some Ascii Data";
   }
}

function validHandler(resource $parser ,string $data) {
}

// Initialise function arguments not being substituted (if any)
$hdl = 'validHandler';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

$fp = fopen(__FILE__, "r");

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

      // string data
      "string",
      'string',

      // object data
      new aClass(),
      
      // resource data
      $fp, 

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for parser

foreach($values as $value) {
      echo @"\nArg value $value \n";
      var_dump( xml_set_element_handler($value, $hdl, $hdl) );
};

fclose($fp);
echo "Done";
?>