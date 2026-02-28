<?hh
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation.
 * Source code: ext/standard/array.c
*/

/*
 * testing krsort() by providing array of hexa-decimal values for $array argument
 * with following flag values:
 *  1.flag value as defualt
 *  2.SORT_REGULAR - compare items normally
 *  3.SORT_NUMERIC - compare items numerically
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing krsort() : usage variations ***\n";

// an array containing unsorted hexadecimal values with keys
$unsorted_hex_array = dict[
  0x1AB => 0x1AB, 0xFFF => 0xFFF, 0xF => 0xF, 0xFF => 0xFF, 0x2AA => 0x2AA, 0xBB => 0xBB,
  0x1ab => 0x1ab, 0xff => 0xff, -0xff => -0xFF, 0 => 0, -0x2aa => -0x2aa
];

echo "\n-- Testing krsort() by supplying hexadecimal value array, 'flag' value is defualt  --\n";
$temp_array = $unsorted_hex_array;
var_dump(krsort(inout $temp_array) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying hexadecimal value array, 'flag' value is SORT_REGULAR  --\n";
$temp_array = $unsorted_hex_array;
var_dump(krsort(inout $temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying hexadecimal value array, 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $unsorted_hex_array;
var_dump(krsort(inout $temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump($temp_array);

echo "Done\n";
}
