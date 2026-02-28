<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
 */

/*
 * Pass array_merge() arrays with mixed keys to test how it attaches them to
 * existing arrays
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : usage variations ***\n";

//mixed keys
$arr1 = dict[0 => 'zero', 20 => 'twenty', 'thirty' => 30, 1 => 'bool'];
$arr2 = dict[0 => 0, 1 => 1, 2 => 2, '' => 'null', (int)1.234E-10 => 'float'];

var_dump(array_merge($arr1, $arr2));
var_dump(array_merge($arr2, $arr1));

echo "Done";
}
