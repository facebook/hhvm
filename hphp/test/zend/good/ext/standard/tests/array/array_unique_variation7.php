<?hh
/* Prototype  : array array_unique(array $input)
 * Description: Removes duplicate values from array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_unique() by passing an array having binary values.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unique() : array with binary data for \$input argument ***\n";

// array with binary values
$input = dict[0 => b"1", 1 => b"hello", 2 => "world", "str1" => "hello", "str2" => "world"];

var_dump( array_unique($input) );

echo "Done";
}
