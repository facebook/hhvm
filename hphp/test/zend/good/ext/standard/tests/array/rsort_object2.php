<?hh
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order
 * Source code: ext/standard/array.c
 */

/*
 * Test functionality of rsort() with objects where properties have different visibilities
 */

// class declaration for integer objects
class for_integer_rsort
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
class for_string_rsort
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
echo "*** Testing rsort() : object functionality ***\n";

// array of integer objects

$unsorted_int_obj = vec[
  new for_integer_rsort(11,33,30),
  new for_integer_rsort(66,44,4),
  new for_integer_rsort(-88,-5,5),
  new for_integer_rsort(0.001,99.5,0.1)
];

// array of string objects
$unsorted_str_obj = varray [
  new for_string_rsort("axx","AXX","ass"),
  new for_string_rsort("t","eee","abb"),
  new for_string_rsort("w","W", "c"),
  new for_string_rsort("py","PY", "pt"),
];


echo "\n-- Sort flag = default --\n";

// testing rsort() function by supplying integer object array, flag value is defualt
$temp_array = $unsorted_int_obj;
var_dump(rsort(inout $temp_array) );
var_dump($temp_array);

// testing rsort() function by supplying string object array, flag value is defualt
$temp_array = $unsorted_str_obj;
var_dump(rsort(inout $temp_array) );
var_dump($temp_array);

echo "\n-- Sort flag = SORT_REGULAR --\n";
// testing rsort() function by supplying integer object array, flag value = SORT_REGULAR
$temp_array = $unsorted_int_obj;
var_dump(rsort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

// testing rsort() function by supplying string object array, flag value = SORT_REGULAR
$temp_array = $unsorted_str_obj;
var_dump(rsort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done";
}
