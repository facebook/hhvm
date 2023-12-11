<?hh
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass an array of different hex values to test how natcasesort() re-orders it
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing natcasesort() : usage variation ***\n";

$unsorted_hex_array = vec[0x1AB, 0xFFF, 0xF, 0xFF, 0x2AA, 0xBB, 0x1ab, 0xff, -0xFF, 0, -0x2aa];
var_dump( natcasesort(inout $unsorted_hex_array) );
var_dump($unsorted_hex_array);

echo "Done";
}
