<?php
/* Prototype  : proto int strspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters found in mask.
		If start or/and length is provided works like strspn(substr($s,$start,$len),$good_chars) 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

error_reporting(E_ALL & ~E_NOTICE);

/*
* Testing strspn() : with different unexpected values for mask argument
*/

echo "*** Testing strspn() : with different unexpected values of mask argument ***\n";

$str = 'string_val';
$start = 1;
$len = 10;


//get an unset variable
$unset_var = 10;
unset ($unset_var);

// declaring class
class sample  {
  public function __toString() {
    return "object";
  }
}

// creating a file resource
$file_handle = fopen(__FILE__, 'r');


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
      new sample(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,

      // resource
      $file_handle
);

// loop through each element of the array for mask

foreach($values as $value) {
      echo "\n-- Iteration with mask value as \"$value\" --\n";
      var_dump( strspn($str,$value) );  // with defalut args
      var_dump( strspn($str,$value,$start) );  // with default len value
      var_dump( strspn($str,$value,$start,$len) );  // with all args
};

// close the resource
fclose($file_handle);

echo "Done"
?>