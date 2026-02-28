<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

function multiply($p, $q) :mixed{
  return ($p * $q);
}

function square($p) :mixed{
  return ($p * $p);
}

function concatenate($a, $b) :mixed{
  return "$a = $b";
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : basic functionality ***\n";

// integer array
$arr1 = vec[1, 2, 3];
$arr2 = vec[4, 5, 6];

echo "-- With two integer array --\n";
var_dump( array_map(multiply<>, $arr1, $arr2) );

echo "-- With single integer array --\n";
var_dump( array_map(square<>, $arr1) );

// string array
$arr1 = vec["one", "two"];
$arr2 = vec["single", "double"];

echo "-- With string array --\n";
var_dump( array_map(concatenate<>, $arr1, $arr2) );

echo "Done";
}
