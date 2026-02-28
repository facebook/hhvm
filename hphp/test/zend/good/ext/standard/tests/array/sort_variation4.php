<?hh
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array.
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * Testing sort() by providing reference variable array with following flag values
 *  flag value as defualt
 *  SORT_REGULAR - compare items normally
 *  SORT_NUMERIC - compare items numerically
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sort() :usage variations  ***\n";

$value1 = 100;
$value2 = 33;
$value3 = 555;


$unsorted_numerics =  vec[ $value1 , $value2, $value3];

echo "\n-- Testing sort() by supplying reference variable array, 'flag' value is defualt --\n";
$temp_array = $unsorted_numerics;
var_dump( sort(inout $temp_array) ); // expecting : bool(true)
var_dump( $temp_array);

echo "\n-- Testing sort() by supplying reference variable array, 'flag' = SORT_REGULAR --\n";
$temp_array = $unsorted_numerics;
var_dump( sort(inout $temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump( $temp_array);

echo "\n-- Testing sort() by supplying reference variable array, 'flag' = SORT_NUMERIC --\n";
var_dump( sort(inout $temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump( $temp_array);

echo "Done\n";
}
