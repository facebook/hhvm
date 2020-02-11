<?hh
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing sort() by providing arrays contains sub arrays for $array argument with flowing flag values
 * flag value as defualt
 * SORT_REGULAR - compare items normally
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sort() : usage variations ***\n";

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
echo "\n-- Testing sort() by supplying various arrays containing sub arrays --\n";

// loop through to test sort() with different arrays
foreach ($various_arrays as $array) {
 
  echo "\n-- Iteration $count --\n"; 
  // testing sort() function by supplying different arrays, flag value is defualt
  echo "- With Defualt sort flag -\n";
  $temp_array = $array;
  var_dump(sort(inout $temp_array) );
  var_dump($temp_array);

  // testing sort() function by supplying different arrays, flag value = SORT_REGULAR
  echo "- Sort flag = SORT_REGULAR -\n";
  $temp_array = $array;
  var_dump(sort(inout $temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done\n";
}
