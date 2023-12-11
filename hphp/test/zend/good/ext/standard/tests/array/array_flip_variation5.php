<?hh
/* Prototype  : array array_flip(array $input)
 * Description: Return array with key <-> value flipped
 * Source code: ext/standard/array.c
*/

/*
* Using different types of repeatitive keys as well as values for 'input' array
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_flip() : 'input' array with repeatitive keys/values ***\n";

// array with numeric key repeatition
$input = dict[1 => 'value', 2 => 'VALUE', 1 => "VaLuE", 3 => 4, 3 => 5];
var_dump( array_flip($input) );

// array with string key repeatition
$input = dict["key" => 1, "two" => 'TWO', 'three' => 3, 'key' => "FOUR"];
var_dump( array_flip($input) );

// array with numeric value repeatition
$input = dict['one' => 1, 'two' => 2, 3 => 1, "index" => 1];
var_dump( array_flip($input) );

//array with string value repeatition
$input = dict['key1' => "value1", "key2" => '2', 'key3' => 'value1'];
var_dump( array_flip($input) );

echo "Done";
}
