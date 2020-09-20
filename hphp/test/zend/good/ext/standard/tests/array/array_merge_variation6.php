<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

/*
 * Pass array_merge arrays with string keys to test behaviour.
 * $arr2 has a duplicate key to $arr1
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : usage variations ***\n";

//string keys
$arr1 = darray['zero' => 'zero', 'one' => 'un', 'two' => 'deux'];
$arr2 = darray['zero' => 'zero', 'un' => 'eins', 'deux' => 'zwei'];

var_dump(array_merge($arr1, $arr2));
var_dump(array_merge($arr2, $arr1));

echo "Done";
}
