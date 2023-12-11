<?hh
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array.
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing sort() by providing integer/string object arrays with flag values are defualt, SORT_REGULAR
*/

// class declaration for integer objects
class for_integer_sort
{
  public $public_class_value;
  private $private_class_value;
  protected $protected_class_value;

  // initializing object member value
  function __construct($value1, $value2,$value3){
    $this->public_class_value = $value1;
    $this->private_class_value = $value2;
    $this->protected_class_value = $value3;
  }
}

// class declaration for string objects
class for_string_sort
{
  public $public_class_value;
  private $private_class_value;
  protected $protected_class_value;
  // initializing object member value
  function __construct($value1, $value2,$value3){
    $this->public_class_value = $value1;
    $this->private_class_value = $value2;
    $this->protected_class_value = $value3;
  }

  // return string value
  function __toString() :mixed{
   return (string)$this->value;
  }

}
<<__EntryPoint>> function main(): void {
echo "*** Testing sort() : object functionality ***\n";

// array of integer objects
$unsorted_int_obj = vec[
  new for_integer_sort(11,33,30),
  new for_integer_sort(66,44,4),
  new for_integer_sort(-88,-5,5),
  new for_integer_sort(0.001,99.5,0.1)
];

// array of string objects
$unsorted_str_obj = varray [
  new for_string_sort("axx","AXX","ass"),
  new for_string_sort("t","eee","abb"),
  new for_string_sort("w","W", "c"),
  new for_string_sort("py","PY", "pt"),
];


echo "\n-- Testing sort() by supplying various object arrays, 'flag' value is defualt --\n";

// testing sort() function by supplying integer object array, flag value is defualt
$temp_array = $unsorted_int_obj;
var_dump(sort(inout $temp_array) );
var_dump($temp_array);

// testing sort() function by supplying string object array, flag value is defualt
$temp_array = $unsorted_str_obj;
var_dump(sort(inout $temp_array) );
var_dump($temp_array);

echo "\n-- Testing sort() by supplying various object arrays, 'flag' value is SORT_REGULAR --\n";
// testing sort() function by supplying integer object array, flag value = SORT_REGULAR
$temp_array = $unsorted_int_obj;
var_dump(sort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

// testing sort() function by supplying string object array, flag value = SORT_REGULAR
$temp_array = $unsorted_str_obj;
var_dump(sort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
}
