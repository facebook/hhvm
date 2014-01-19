<?php
/* Prototype  : proto bool class_exists(string classname [, bool autoload])
 * Description: Checks if the class exists 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

function __autoload($className) {
	echo "In __autoload($className)\n";
}

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
set_error_handler('test_error_handler');

echo "*** Testing class_exists() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$autoload = true;

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
      new stdclass(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for classname

foreach($values as $value) {
      echo "\nArg value $value \n";
      var_dump( class_exists($value, $autoload) );
};

echo "Done";
?>