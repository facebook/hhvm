<?hh
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing sort() by providing different hexa-decimal array for $array argument with following flag values
 * flag  value as defualt
 * SORT_REGULAR - compare items normally
 * SORT_NUMERIC - compare items numerically
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sort() : usage variations ***\n";

// an array contains unsorted hexadecimal values  
$unsorted_hex_array = vec[0x1AB, 0xFFF, 0xF, 0xFF, 0x2AA, 0xBB, 0x1ab, 0xff, -0xFF, 0, -0x2aa];

echo "\n-- Testing sort() by supplying hexadecimal value array, 'flag' value is defualt  --\n";
$temp_array = $unsorted_hex_array;
var_dump(sort(inout $temp_array) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing sort() by supplying hexadecimal value array, 'flag' value is SORT_REGULAR  --\n";
$temp_array = $unsorted_hex_array;
var_dump(sort(inout $temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing sort() by supplying hexadecimal value array, 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $unsorted_hex_array;
var_dump(sort(inout $temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump($temp_array);

echo "Done\n";
}
