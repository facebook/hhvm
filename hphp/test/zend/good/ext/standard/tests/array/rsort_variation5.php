<?hh
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays containing different string data to rsort() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing rsort() : variation ***\n";

$various_arrays = vec[
// group of escape sequences
vec["\a", "\cx", "\e", "\f", "\n", "\t", "\xhh", "\ddd", "\v"],

// array contains combination of capital/small letters
vec["lemoN", "Orange", "banana", "apple", "Test", "TTTT", "ttt", "ww", "x", "X", "oraNGe", "BANANA"]
];

$flags = dict["SORT_REGULAR" => SORT_REGULAR, "SORT_STRING" => SORT_STRING];

$count = 1;
// loop through to test rsort() with different arrays
foreach ($various_arrays as $array) {
    echo "\n-- Iteration $count --\n";

    echo "- With Default sort flag -\n";
    $temp_array = $array;
    var_dump(usort(inout $temp_array,  ($a, $b) ==> -HH\Lib\Legacy_FIXME\cmp($a, $b)) );
    var_dump($temp_array);

    // loop through $flags array and setting all possible flag values
    foreach($flags as $key => $flag){
        echo "- Sort flag = $key -\n";

        $temp_array = $array;
        var_dump(rsort(inout $temp_array, $flag) );
        var_dump($temp_array);
    }
    $count++;
}

echo "Done";
}
