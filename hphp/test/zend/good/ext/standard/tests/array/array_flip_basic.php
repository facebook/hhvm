<?hh
/* Prototype  : array array_flip(array $input)
 * Description: Return array with key <-> value flipped
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_flip() : basic functionality ***\n";

// array with default keys - numeric values
$input = vec[1, 2];
var_dump( array_flip($input) );

// array with default keys - string values
$input = vec['value1', "value2"];
var_dump( array_flip($input) );

// associative arrays - key as string
$input = dict['key1' => 1, "key2" => 2];
var_dump( array_flip($input) );

// associative arrays - key as numeric
$input = dict[1 => 'one', 2 => "two"];
var_dump( array_flip($input) );

// combination of associative and non-associative array
$input = dict[1 => 'one', 2 => 'two', 3 => 'three', 4 => 4, "five" => 5];
var_dump( array_flip($input) );
echo "Done";
}
