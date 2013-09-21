<?php
/* Prototype  : bool ksort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key, maintaining key to data correlation.  
 * Source code: ext/standard/array.c
*/
/*
 * testing ksort() by providing array ofinteger/string objects with following flag values: 
 *  1.SORT_NUMERIC - compare items numerically
 *  2.SORT_STRING - compare items as strings 
*/

echo "*** Testing ksort() : object functionality ***\n";

// class declaration for integer objects
class Integer
{
  public $class_value;
  // initializing object member value
  function __construct($value){
    $this->class_value = $value;
  }

}

// class declaration for string objects
class String
{
  public $class_value;
  // initializing object member value
  function __construct($value){
    $this->class_value = $value;
  }

  // return string value
  function __tostring() {
   return (string)$this->value;
  }

}

// array of integer objects
$unsorted_int_obj = array ( 
  11 => new Integer(11), 66 =>  new Integer(66),
  23 => new Integer(23), -5 => new Integer(-5),
  1 => new Integer(0.001), 0 => new Integer(0)
);

// array of string objects
$unsorted_str_obj = array ( 
  "axx" => new String("axx"), "t" => new String("t"),
  "w" => new String("w"), "py" => new String("py"),
  "apple" => new String("apple"), "Orange" => new String("Orange"),
  "Lemon" => new String("Lemon"), "aPPle" => new String("aPPle")
);
echo "\n-- Testing ksort() by supplying various object arrays, 'flag' value is defualt --\n";

// testing ksort() function by supplying integer object array, flag value is defualt
$temp_array = $unsorted_int_obj;
var_dump(ksort($temp_array) );
var_dump($temp_array);

// testing ksort() function by supplying string object array, flag value is defualt
$temp_array = $unsorted_str_obj;
var_dump(ksort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing ksort() by supplying various object arrays, 'flag' value is SORT_REGULAR --\n";
// testing ksort() function by supplying integer object array, flag value = SORT_REGULAR
$temp_array = $unsorted_int_obj;
var_dump(ksort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

// testing ksort() function by supplying string object array, flag value = SORT_REGULAR
$temp_array = $unsorted_str_obj;
var_dump(ksort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
?>