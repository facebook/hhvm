<?php
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation  
 * Source code: ext/standard/array.c
*/
/*
 * testing krsort() by providing array of integer/string objects with following flag values:
 *  1.Defualt flag value
 *  2.SORT_REGULAR - compare items normally
*/

echo "*** Testing krsort() : object functionality ***\n";

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

// array of integer objects with different key values
$unsorted_int_obj = array ( 
  10 => new Integer(11), 20 =>  new Integer(66),
  3 => new Integer(23), 4 => new Integer(-5),
  50 => new Integer(0.001), 6 => new Integer(0)
);

// array of string objects with different key values
$unsorted_str_obj = array ( 
  "axx" => new String("axx"), "t" => new String("t"),
  "w" => new String("w"), "py" => new String("py"),
  "apple" => new String("apple"), "Orange" => new String("Orange"),
  "Lemon" => new String("Lemon"), "aPPle" => new String("aPPle")
);


echo "\n-- Testing krsort() by supplying various object arrays, 'flag' value is defualt --\n";

// testing krsort() function by supplying integer object array, flag value is defualt
$temp_array = $unsorted_int_obj;
var_dump(krsort($temp_array) );
var_dump($temp_array);

// testing krsort() function by supplying string object array, flag value is defualt
$temp_array = $unsorted_str_obj;
var_dump(krsort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying various object arrays, 'flag' value is SORT_REGULAR --\n";
// testing krsort() function by supplying integer object array, flag value = SORT_REGULAR
$temp_array = $unsorted_int_obj;
var_dump(krsort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

// testing krsort() function by supplying string object array, flag value = SORT_REGULAR
$temp_array = $unsorted_str_obj;
var_dump(krsort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
?>