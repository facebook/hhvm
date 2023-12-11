<?hh
/* Prototype  : bool arsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array and maintain index association.
                Elements will be arranged from highest to lowest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * Testing arsort() with following flag values
 *  flag value as defualt
 *  SORT_REGULAR - compare items normally
 *  SORT_NUMERIC - compare items numerically
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing arsort() :usage variations  ***\n";

$value1 = 100;
$value2 = 33;
$value3 = 555;


$unsorted_numerics =  dict[ 1 => $value1 , 2 => $value2, 3 => $value3];

echo "\n-- Testing arsort() 'flag' value is defualt --\n";
$temp_array = $unsorted_numerics;
var_dump( arsort(inout $temp_array) ); // expecting : bool(true)
var_dump( $temp_array);

echo "\n-- Testing arsort() 'flag' = SORT_REGULAR --\n";
$temp_array = $unsorted_numerics;
var_dump( arsort(inout $temp_array, SORT_REGULAR) ); // expecting : bool(true)
var_dump( $temp_array);

echo "\n-- Testing arsort() 'flag' = SORT_NUMERIC --\n";
var_dump( arsort(inout $temp_array, SORT_NUMERIC) ); // expecting : bool(true)
var_dump( $temp_array);

echo "Done\n";
}
