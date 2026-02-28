<?hh
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass an array of octal values to test how natcasesort() re-orders it
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing natcasesort() : usage variation ***\n";

$unsorted_oct_array = vec[01235, 0321, 0345, 066, 0772, 077, -066, -0345, 0];

var_dump( natcasesort(inout $unsorted_oct_array) );
var_dump($unsorted_oct_array);

echo "Done";
}
