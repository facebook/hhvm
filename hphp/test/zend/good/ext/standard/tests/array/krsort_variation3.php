<?hh
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation
 * Source code: ext/standard/array.c
*/

/*
 * Testing krsort() by providing array of integer/float/mixed values for $array argument
 * with following flag values:
 *  1.flag value as defualt
 *  2.SORT_REGULAR - compare items normally
 *  3.SORT_NUMERIC - compare items numerically
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing krsort() : usage variations ***\n";

// diff. associative arrays to sort
$various_arrays = vec[
  // negative/posative integer key value array
  dict[1 => 11, -2 => -11, 3 => 21, -4 => -21, 5 => 31, -6 => -31, 7 => 0, 8 => 41, -10 =>-41],

  // float key values
  dict[1 => 10.5, 0 => -10.5, 3 => 10.5e2, 4 => 10.6E-2, 0 => .5, 6 => .0001, -7 => -.1],

  // mixed value array with different types of keys
  dict[1 => .0001, 2 => .0021, -3 => -.01, 4 => -1, 5 => 0, 6 => .09, 7 => 2, -8 => -.9, 9 => 10.6E-2,
        -10 => -10.6E-2, 11 => 33]
];

// set of possible flag values
$flags = dict["SORT_REGULAR" => SORT_REGULAR, "SORT_NUMERIC" => SORT_NUMERIC];

$count = 1;
echo "\n-- Testing krsort() by supplying various integer/float arrays --\n";

// loop through to test krsort() with different arrays
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With defualt sort flag -\n";
  $temp_array = $array;
  var_dump(krsort(inout $temp_array) );
  var_dump($temp_array);

  // loop through $flags array and call krsort() with all possible sort flag values
  foreach($flags as $key => $flag){
    echo "- Sort flag = $key -\n";
    $temp_array = $array;
    var_dump(krsort(inout $temp_array, $flag) );
    var_dump($temp_array);
  }
  $count++;
}

echo "Done\n";
}
