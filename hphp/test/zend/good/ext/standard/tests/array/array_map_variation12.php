<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing buit-in function as callback function
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : built-in function ***\n";

$array1 = vec[1, 2, 3];
$array2 = vec[3, 4, 5];

echo "-- with built-in function 'pow' and two parameters --\n";
var_dump( array_map('pow', $array1, $array2));

echo "-- with built-in function 'pow' and one parameter --\n";
try { var_dump( array_map('pow', $array1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "-- with language construct --\n";
var_dump( array_map('echo', $array1));

echo "Done";
}
