<?hh
/* Prototype  : array array_diff_ukey(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments. User supplied function is used for comparing the keys. This function is like array_udiff() but works on the keys instead of the values. The associativity is preserved.
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_ukey() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$array1 = darray[

      'first' => darray['blue' => 1, 'red' => 2],

      'second' => darray['yellow' => 7],

      'third' => darray[0 => 'zero'],
];

$array2 = darray [

      'first' => darray['blue' => 1, 'red' => 2,],

      'second' => darray['cyan' => 8],

      'fourth' => darray[2 => 'two'],
];

echo "\n-- Testing array_diff_ukey() function with multi dimensional array --\n";
var_dump( array_diff_ukey($array1, $array2, fun('strcasecmp')) );
var_dump( array_diff_ukey($array2, $array1, fun('strcasecmp')) );
echo "===DONE===\n";
}
