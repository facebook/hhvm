<?hh
/* Prototype  : bool arsort ( array &$array [, int $asort_flags] )
 * Description: Sort an array and maintain index association.
                Elements will be arranged from highest to lowest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing arsort() by providing different hexa-decimal array for $array argument with following flag values
 * flag value as defualt
 * SORT_REGULAR - compare items normally
 * SORT_NUMERIC - compare items numerically
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing arsort() : usage variations ***\n";

// an array contains unsorted hexadecimal values
// There are multiple keys which are duplicate and the later should be picked
$unsorted_hex_array = darray [ 0x1AB => 0x1AB, 0xFFF => 0xFFF, 0xF => 0xF, 0xFF => 0xFF, 0x2AA => 0x2AA, 0xBB => 0xBB,
                              0x1ab => 0x1ab, 0xff => 0xff, -0xff => -0xFF, 0 => 0, -0x2aa => -0x2aa
                            ];

echo "\n-- Testing arsort() by supplying hexadecimal value array, 'flag' value is defualt  --\n";
$temp_array = $unsorted_hex_array;
var_dump(arsort(inout $temp_array) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing arsort() by supplying hexadecimal value array, 'flag' value is SORT_REGULAR  --\n";
$temp_array = $unsorted_hex_array;
var_dump(arsort(inout $temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing arsort() by supplying hexadecimal value array, 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $unsorted_hex_array;
var_dump(arsort(inout $temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump($temp_array);

echo "Done\n";
}
