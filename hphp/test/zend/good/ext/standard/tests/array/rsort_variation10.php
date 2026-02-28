<?hh
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order
 * Source code: ext/standard/array.c
 */

/*
 * Pass rsort() an array containing octal values to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing rsort() : variation ***\n";

// an array containing unsorted octal values
$unsorted_oct_array = vec[01235, 0321, 0345, 066, 0772, 077, -066, -0345, 0];

echo "\n-- Sort flag = default  --\n";
$temp_array = $unsorted_oct_array;
var_dump(rsort(inout $temp_array) );
var_dump($temp_array);

echo "\n-- Sort flag = SORT_REGULAR  --\n";
$temp_array = $unsorted_oct_array;
var_dump(rsort(inout $temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "\n-- Sort flag = SORT_NUMERIC  --\n";
$temp_array = $unsorted_oct_array;
var_dump(rsort(inout $temp_array, SORT_NUMERIC) );
var_dump($temp_array);

echo "Done";
}
