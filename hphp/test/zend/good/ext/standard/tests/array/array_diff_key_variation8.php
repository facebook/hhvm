<?hh
/* Prototype  : array array_diff_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments.
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_key() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$array1 = dict[

      'first' => dict['blue' => 1, 'red' => 2],

      'second' => dict['yellow' => 7],

      'third' => dict[0 => 'zero'],
];

$array2 = darray [

      'first' => dict['blue' => 1, 'red' => 2,],

      'second' => dict['cyan' => 8],

      'fourth' => dict[2 => 'two'],
];

echo "\n-- Testing array_diff_key() function with multi dimensional array --\n";
var_dump( array_diff_key($array1, $array2) );
var_dump( array_diff_key($array2, $array1) );
echo "===DONE===\n";
}
