<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */
function callback2($p, $q) :mixed{
  return $p * $q;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : error conditions ***\n";

echo "\n-- Testing array_map() function with less no. of arrays than callback function arguments --\n";
$arr1 = vec[1, 2];
try { var_dump( array_map(callback2<>, $arr1) ); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "\n-- Testing array_map() function with more no. of arrays than callback function arguments --\n";
$arr2 = vec[3, 4];
$arr3 = vec[5, 6];
var_dump( array_map(callback2<>, $arr1, $arr2, $arr3) );

echo "Done";
}
