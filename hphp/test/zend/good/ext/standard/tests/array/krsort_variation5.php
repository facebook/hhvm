<?hh
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation
 * Source code: ext/standard/array.c
*/

/*
 * testing krsort() by providing array of string values for $array argument with
 * following flag values:
 *  1.flag value as defualt
 *  2.SORT_REGULAR - compare items normally
 *  3.SORT_STRING  - compare items as strings
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing krsort() : usage variations ***\n";

$various_arrays = vec[
  // diff. escape sequence chars with key values
  dict[ '' =>  null, '' => NULL, "\a" => "\a", "\cx" => "\cx", "\e" => "\e",
          "\f" => "\f", "\n" =>"\n", "\r" => "\r", "\t" => "\t", "\xhh" => "\xhh",
          "\ddd" => "\ddd", "\v" => "\v"
        ],

  // array containing different strings with key values
  dict[ 'Lemon' => "lemoN", 'o' => "Orange", 'B' => "banana", 'Apple' => "apple", 'te' => "Test",
          't' => "TTTT", 'T' => "ttt", 'W' => "ww", 'X' => "x", 'x' => "X", 'O' => "oraNGe",
          'B' => "BANANA"
        ]
];

$flags = dict["SORT_REGULAR" => SORT_REGULAR, "SORT_STRING" => SORT_STRING];

$count = 1;
echo "\n-- Testing krsort() by supplying various string arrays --\n";

// loop through to test krsort() with different arrays
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With defualt sort flag -\n";
  $temp_array = $array;
  var_dump(krsort(inout $temp_array) ); // expecting : bool(true)
  var_dump($temp_array);

  // loop through $flags array and call krsort() with all possible sort flag values
  foreach($flags as $key => $flag){
    echo "- Sort flag = $key -\n";
    $temp_array = $array;
    var_dump(krsort(inout $temp_array, $flag) ); // expecting : bool(true)
    var_dump($temp_array);
  }
  $count++;
}

echo "Done\n";
}
