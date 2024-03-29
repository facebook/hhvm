<?hh
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass natcasesort() an array of objects to test how it re-orders them
 */

// class declaration for string objects
class for_string_natcasesort
{
  public $class_value;
  // initializing object member value
  function __construct($value){
    $this->class_value = $value;
  }

  // return string value
  function __toString() :mixed{
   return (string)$this->class_value;
  }

}

<<__EntryPoint>> function main(): void {
echo "*** Testing natcasesort() : object functionality ***\n";


// array of string objects
$unsorted_str_obj = vec[
  new for_string_natcasesort("axx"), new for_string_natcasesort("t"),
  new for_string_natcasesort("w"), new for_string_natcasesort("py"),
  new for_string_natcasesort("apple"), new for_string_natcasesort("Orange"),
  new for_string_natcasesort("Lemon"), new for_string_natcasesort("aPPle")
];


echo "\n-- Testing natcasesort() by supplying various object arrays --\n";

// testing natcasesort() function by supplying string object array
var_dump(natcasesort(inout $unsorted_str_obj) );
var_dump($unsorted_str_obj);

echo "Done";
}
