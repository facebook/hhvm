<?hh
/* Prototype  : array array_intersect_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are present in all the other arguments.
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_key() : usage variation ***\n";

/// Initialise function arguments not being substituted (if any)
$array1 = dict[

      'first' => dict['blue'  => 1, 'red'  => 2],

      'second' => dict['yellow' => 7],

      'third' => dict[0 =>'zero'],
];

$array2 = darray [

      'first' => dict['blue'  => 1, 'red'  => 2,],

      'second' => dict['cyan'   => 8],

      'fourth' => dict[2 => 'two'],
];
var_dump( array_intersect_key($array1, $array2) );
var_dump( array_intersect_key($array2,$array1 ) );
echo "===DONE===\n";
}
