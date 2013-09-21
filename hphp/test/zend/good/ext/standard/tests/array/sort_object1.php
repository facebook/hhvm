<?php
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/
/*
 * testing sort() by providing integer/string object arrays with flag values are defualt, SORT_REGULAR
*/

echo "*** Testing sort() : object functionality ***\n";

// class declaration for integer objects
class for_integer_sort
{
  public $class_value;
  // initializing object member value
  function __construct($value){
    $this->class_value = $value;
  }

}

// class declaration for string objects
class for_string_sort
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
$unsorted_int_obj = array( 
  new for_integer_sort(11), new for_integer_sort(66),
  new for_integer_sort(23), new for_integer_sort(-5),
  new for_integer_sort(0.001), new for_integer_sort(0)
);

// array of string objects
$unsorted_str_obj = array ( 
  new for_string_sort("axx"), new for_string_sort("t"),
  new for_string_sort("w"), new for_string_sort("py"),
  new for_string_sort("apple"), new for_string_sort("Orange"),
  new for_string_sort("Lemon"), new for_string_sort("aPPle")
);


echo "\n-- Testing sort() by supplying various object arrays, 'flag' value is defualt --\n";

// testing sort() function by supplying integer object array, flag value is defualt
$temp_array = $unsorted_int_obj;
var_dump(sort($temp_array) );
var_dump($temp_array);

// testing sort() function by supplying string object array, flag value is defualt
$temp_array = $unsorted_str_obj;
var_dump(sort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing sort() by supplying various object arrays, 'flag' value is SORT_REGULAR --\n";
// testing sort() function by supplying integer object array, flag value = SORT_REGULAR
$temp_array = $unsorted_int_obj;
var_dump(sort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

// testing sort() function by supplying string object array, flag value = SORT_REGULAR
$temp_array = $unsorted_str_obj;
var_dump(sort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
?>