<?hh
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order
 * Source code: ext/standard/array.c
 */

/*
 * Test behaviour of rsort() when:
 * 1. passed an array of referenced variables
 * 2. $array_arg is a reference to another array
 * 3. $array_arg is passed by reference
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing rsort() : variation ***\n";

$value1 = 100;
$value2 = 33;
$value3 = 555;


$unsorted_numerics =  vec[ $value1 , $value2, $value3];

echo "\n-- 'flag' value is defualt --\n";
$temp_array = $unsorted_numerics;
var_dump( rsort(inout $temp_array) );
var_dump( $temp_array);

echo "\n-- 'flag' = SORT_REGULAR --\n";
$temp_array = $unsorted_numerics;
var_dump( rsort(inout $temp_array, SORT_REGULAR) );
var_dump( $temp_array);

echo "Done";
}
