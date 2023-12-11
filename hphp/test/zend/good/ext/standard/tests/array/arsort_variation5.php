<?hh
/* Prototype  : bool arsort ( array &$array [, int $asort_flags] )
 * Description: Sort an array and maintain index association
                Elements will be arranged from highest to lowest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing arsort() by providing different string arrays for $array argument with following flag values
 *  flag value as defualt
 *  SORT_REGULAR - compare items normally
 *  SORT_STRING  - compare items as strings
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing arsort() : usage variations ***\n";

$various_arrays = varray [
  // group of escape sequences
  darray ["\a" => "\a", "\cx" => "\cx", "\e" => "\e",
        "\f" => "\f", "\n" =>"\n", "\r" => "\r", "\t" => "\t", "\xhh" => "\xhh",
        "\ddd" => "\ddd", "\v" => "\v"
        ],

  // array contains combination of capital/small letters
  darray ['l' => "lemoN", 'O' => "Orange", 'b' => "banana", 'a' => "apple", 'Te' => "Test",
        'T' => "TTTT", 't' => "ttt", 'w' => "ww", 'x' => "x", 'X' => "X", 'o' => "oraNGe",
        'B' => "BANANA"
        ]
];

$flags = dict["SORT_REGULAR" => SORT_REGULAR, "SORT_STRING" => SORT_STRING];

$count = 1;
echo "\n-- Testing arsort() by supplying various string arrays --\n";

// loop through to test arsort() with different arrays
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With default sort_flag -\n";
  $temp_array = $array;
  var_dump(uasort(inout $temp_array,  ($a, $b) ==> -HH\Lib\Legacy_FIXME\cmp($a, $b)) ); // expecting : bool(true)
  var_dump($temp_array);

  // loop through $flags array and setting all possible flag values
  foreach($flags as $key => $flag){
    echo "- Sort_flag = $key -\n";
    $temp_array = $array;
    var_dump(arsort(inout $temp_array, $flag) ); // expecting : bool(true)
    var_dump($temp_array);
  }
  $count++;
}

echo "Done\n";
}
