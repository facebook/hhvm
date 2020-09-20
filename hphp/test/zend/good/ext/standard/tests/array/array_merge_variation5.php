<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

/*
 * Pass array_merge() arrays with only numeric keys to test behaviour.
 * $arr2 contains a duplicate element to $arr1.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : usage variations ***\n";

//numeric keys
$arr1 = varray['zero', 'one', 'two', 'three'];
$arr2 = darray[1 => 'one', 20 => 'twenty', 30 => 'thirty'];

var_dump(array_merge($arr1, $arr2));
var_dump(array_merge($arr2, $arr1));

echo "Done";
}
