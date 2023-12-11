<?hh
/* Prototype  : array array_intersect(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments
 * Source code: ext/standard/array.c
*/

/*
* Testing the behavior of array_intersect() by passing array with
* binary values for $arr1 and $arr2 argument.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect() : binary safe checking ***\n";

// array with binary values
$arr_binary = vec[b"hello", b"world"];
// simple array
$arr_normal = vec["hello", "world"];

// array with binary value for $arr1 argument
var_dump( array_intersect($arr_binary, $arr_normal) );

// array with binary value for $arr2 argument
var_dump( array_intersect($arr_normal, $arr_binary) );

// array with binary value for both $arr1 and $arr2 argument
var_dump( array_intersect($arr_binary, $arr_binary) );

echo "Done";
}
