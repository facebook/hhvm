<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of arr1 that have values which are not
 * present in any of the others arguments but do additional checks whether
 * the keys are equal
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff_assoc() compares binary data
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_assoc() : usage variations ***\n";

$array1 = dict[0 => b"1",
                 1 => b"hello",
                 2 => "world",
                 "str1" => "hello",
                 "str2" => "world"];

$array2 = dict[b"1" => 'hello',
                 0 => b"world",
                 1 => "hello",
                 2 => 'test'];

var_dump(array_diff_assoc($array1, $array2));
var_dump(array_diff_assoc($array2, $array1));

echo "Done";
}
