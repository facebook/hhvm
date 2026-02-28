<?hh
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass natcasesort() an array of objects which have properties of different
 * visibilities to test how it re-orders the array.
 */

// class declaration for string objects
class for_string_natcasesort
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
        return (string)$this->public_class_value;
    }

}
<<__EntryPoint>> function main(): void {
echo "*** Testing natcasesort() : object functionality ***\n";

// array of string objects
$unsorted_str_obj = vec[
new for_string_natcasesort("axx","AXX","ass"),
new for_string_natcasesort("t","eee","abb"),
new for_string_natcasesort("w","W", "c"),
new for_string_natcasesort("py","PY", "pt"),
];


echo "\n-- Testing natcasesort() by supplying object arrays --\n";

// testing natcasesort() function by supplying string object array
$temp_array = $unsorted_str_obj;
var_dump(natcasesort(inout $temp_array) );
var_dump($temp_array);

echo "Done";
}
