<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are
 * not present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

/*
 * Test behaviour of array_diff() function with binary input
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : usage variations ***\n";


$array1 = dict[ 0 => b"1",
                  1 => b"hello",
                  2 => "world",
                  "str1" => "hello",
                  "str2" => "world" ];

$array2 = dict[ b"1" => 'hello',
                  0 => b"world",
                  1 => "hello",
                  2 => 'test' ];

var_dump(array_diff($array1, $array2));
var_dump(array_diff($array2, $array1));

echo "Done";
}
