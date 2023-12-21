<?hh
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation
 * Source code: ext/standard/array.c
*/
/*
 * testing krsort() by providing array of integer/string objects with following flag values:
 *  1.Defualt flag value
 *  2.SORT_REGULAR - compare items normally
*/

// class declaration for integer objects
class MyInteger
{
  public $class_value;
  // initializing object member value
  function __construct($value){
    $this->class_value = $value;
  }
}

// class declaration for string objects
class MyString
{
  public $class_value;
  // initializing object member value
  function __construct($value){
    $this->class_value = $value;
  }

  // return string value
  function __toString() :mixed{
   return (string)$this->value;
  }

}
<<__EntryPoint>> function main(): void {
echo "*** Testing krsort() : object functionality ***\n";

// array of integer objects with different key values
$unsorted_int_obj = dict[
  10 => new MyInteger(11), 20 =>  new MyInteger(66),
  3 => new MyInteger(23), 4 => new MyInteger(-5),
  50 => new MyInteger(0.001), 6 => new MyInteger(0)
];

// array of string objects with different key values
$unsorted_str_obj = dict[
  "axx" => new MyString("axx"), "t" => new MyString("t"),
  "w" => new MyString("w"), "py" => new MyString("py"),
  "apple" => new MyString("apple"), "Orange" => new MyString("Orange"),
  "Lemon" => new MyString("Lemon"), "aPPle" => new MyString("aPPle")
];


echo "\n-- Testing krsort() by supplying various object arrays, 'flag' value is defualt --\n";

// testing krsort() function by supplying integer object array, flag value is defualt
$temp_array = $unsorted_int_obj;
var_dump(krsort(inout $temp_array) );
var_dump($temp_array);

// testing krsort() function by supplying string object array, flag value is defualt
$temp_array = $unsorted_str_obj;
var_dump(krsort(inout $temp_array) );
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying various object arrays, 'flag' value is SORT_REGULAR --\n";
// testing krsort() function by supplying integer object array, flag value = SORT_REGULAR
$temp_array = $unsorted_int_obj;
var_dump(krsort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

// testing krsort() function by supplying string object array, flag value = SORT_REGULAR
$temp_array = $unsorted_str_obj;
var_dump(krsort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
}
