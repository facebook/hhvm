<?hh
/*
 * Prototype  : mixed array_search ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns the key if it is found in the array, FALSE otherwise
 * Source Code: ext/standard/array.c
*/

class array_search_check {
  public $array_var = dict[1=>"one", "two"=>2, 3=>3];
  public function foo() :mixed{
    echo "Public function\n";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing sub-arrays with array_search() ***\n";
$sub_array = dict[
  0 => "one",
  1 => dict[0 => 1, 2 => "two", "three" => 3],
  4 => "four",
  "five" => 5,
  5 => vec['', 'i']
];
var_dump( array_search("four", $sub_array) );
//checking for element in a sub-array
var_dump( array_search(3, $sub_array[1]) );
var_dump( array_search(vec['','i'], $sub_array) );

/* checking for objects in array_search() */
echo "\n*** Testing objects with array_search() ***\n";

$array_search_obj = new array_search_check();  //creating new object
//error: as wrong datatype for second argument
var_dump( array_search("array_var", $array_search_obj) );
//error: as wrong datatype for second argument
var_dump( array_search("foo", $array_search_obj) );
//element found as "one" exists in array $array_var
var_dump( array_search("one", $array_search_obj->array_var) );

echo "Done\n";
}
