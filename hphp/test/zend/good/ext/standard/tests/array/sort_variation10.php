<?hh
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array.
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing sort() by providing different octal array for $array argument
 * with following flag values
 * 1.flag value as defualt
 * 2.SORT_REGULAR - compare items normally
 * 3.SORT_NUMERIC - compare items numerically
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sort() : usage variations ***\n";

// an array containing unsorted octal values
$unsorted_oct_array = vec[01235, 0321, 0345, 066, 0772, 077, -066, -0345, 0];

echo "\n-- Testing sort() by supplying octal value array, 'flag' value is defualt  --\n";
$temp_array = $unsorted_oct_array;
var_dump(sort(inout $temp_array) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing sort() by supplying octal value array, 'flag' value is SORT_REGULAR  --\n";
$temp_array = $unsorted_oct_array;
var_dump(sort(inout $temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump($temp_array);

echo "\n-- Testing sort() by supplying octal value array, 'flag' value is SORT_NUMERIC  --\n";
$temp_array = $unsorted_oct_array;
var_dump(sort(inout $temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump($temp_array);

echo "Done\n";
}
