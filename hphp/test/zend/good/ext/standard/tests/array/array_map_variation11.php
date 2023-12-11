<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing subarrays and recursive callback function
 */

// callback function
function square_recur_single_array($var) :mixed{
   if (is_array($var))
     return array_map(square_recur_single_array<>, $var);
   return $var * $var;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : recursive callback function ***\n";

$array1 = vec[1, vec[2, 3, vec[5]], vec[4]];

var_dump( array_map(square_recur_single_array<>, $array1));

echo "Done";
}
