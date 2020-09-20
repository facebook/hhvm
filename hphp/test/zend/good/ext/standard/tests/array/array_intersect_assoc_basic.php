<?hh
/* Prototype  : array array_intersect_assoc(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments.
 * Keys are used to do more restrictive check
 * Source code: ext/standard/array.c
*/

/*
* Testing the behavior of array_intersect_assoc() by passing different arrays for the arguments.
* Function is tested by passing associative array as well as array with default keys.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_assoc() : basic functionality ***\n";

// array with default keys
$arr_default_keys = varray[1, 2, "hello", 'world'];

// associative array
$arr_associative = darray["one" => 1, "two" => 2];

// default key array for both $arr1 and $arr2 argument
var_dump( array_intersect_assoc($arr_default_keys, $arr_default_keys) );

// default key array for $arr1 and associative array for $arr2 argument
var_dump( array_intersect_assoc($arr_default_keys, $arr_associative) );

// associative array for $arr1 and default key array for $arr2
var_dump( array_intersect_assoc($arr_associative, $arr_default_keys) );

// associative array for both $arr1 and $arr2 argument
var_dump( array_intersect_assoc($arr_associative, $arr_associative) );

// more arrays to be intersected
$arr3 = varray[2, 3, 4];
var_dump( array_intersect_assoc($arr_default_keys, $arr_associative, $arr3) );
var_dump( array_intersect_assoc($arr_associative, $arr_default_keys, $arr3, $arr_associative) );

echo "Done";
}
