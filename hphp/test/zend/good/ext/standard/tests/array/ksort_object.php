<?hh
/* Prototype  : bool ksort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key, maintaining key to data correlation.
 * Source code: ext/standard/array.c
*/
/*
 * testing ksort() by providing array ofinteger/string objects with following flag values:
 *  1.SORT_NUMERIC - compare items numerically
 *  2.SORT_STRING - compare items as strings
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
echo "*** Testing ksort() : object functionality ***\n";

// array of integer objects
$unsorted_int_obj = dict[
  11 => new MyInteger(11), 66 =>  new MyInteger(66),
  23 => new MyInteger(23), -5 => new MyInteger(-5),
  1 => new MyInteger(0.001), 0 => new MyInteger(0)
];

// array of string objects
$unsorted_str_obj = dict[
  "axx" => new MyString("axx"), "t" => new MyString("t"),
  "w" => new MyString("w"), "py" => new MyString("py"),
  "apple" => new MyString("apple"), "Orange" => new MyString("Orange"),
  "Lemon" => new MyString("Lemon"), "aPPle" => new MyString("aPPle")
];
echo "\n-- Testing ksort() by supplying various object arrays, 'flag' value is defualt --\n";

// testing ksort() function by supplying integer object array, flag value is defualt
$temp_array = $unsorted_int_obj;
var_dump(ksort(inout $temp_array) );
var_dump($temp_array);

// testing ksort() function by supplying string object array, flag value is defualt
$temp_array = $unsorted_str_obj;
var_dump(ksort(inout $temp_array) );
var_dump($temp_array);

echo "\n-- Testing ksort() by supplying various object arrays, 'flag' value is SORT_REGULAR --\n";
// testing ksort() function by supplying integer object array, flag value = SORT_REGULAR
$temp_array = $unsorted_int_obj;
var_dump(ksort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

// testing ksort() function by supplying string object array, flag value = SORT_REGULAR
$temp_array = $unsorted_str_obj;
var_dump(ksort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
}
