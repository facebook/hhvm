<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing anoymous callback function with following variations
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : anonymous callback function ***\n";

$array1 = vec[1, 2, 3];
$array2 = vec[3, 4, 5];

echo "-- anonymous function with all parameters and body --\n";
var_dump( array_map( ($a, $b) ==> vec[$a, $b], $array1, $array2));

echo "-- anonymous function with two parameters and passing one array --\n";
try { var_dump( array_map( ($a, $b) ==> vec[$a, $b], $array1)); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "-- anonymous function with NULL parameter --\n";
var_dump( array_map( () ==> null, $array1));

echo "-- anonymous function with NULL body --\n";
var_dump( array_map( $a ==> {}, $array1));

echo "-- passing NULL as 'arr1' --\n";
var_dump( array_map( $a ==> vec[$a], NULL));

echo "Done";
}
