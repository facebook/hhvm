<?hh
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order
 * Source code: ext/standard/array.c
 */

/*
 * Pass rsort() multi-dimensional arrays to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing rsort() : variation ***\n";

// array of arrays
$various_arrays = varray [
  // null array
  varray[],

  // array contains null sub array
  varray[ varray[] ],

  // array of arrays along with some values
  varray[44, 11, varray[64, 61] ],

  // array containing sub arrays
  varray[varray[33, -5, 6], varray[11], varray[22, -55], varray[] ]
];


$count = 1;

// loop through to test rsort() with different arrays
foreach ($various_arrays as $array) {

  echo "\n-- Iteration $count --\n";

  echo "\n-- 'flag' value is default --\n";
  $temp_array = $array;
  var_dump(rsort(inout $temp_array) );
  var_dump($temp_array);

  echo "\n-- 'flag' value is SORT_REGULAR --\n";
  $temp_array = $array;
  var_dump(rsort(inout $temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done";
}
