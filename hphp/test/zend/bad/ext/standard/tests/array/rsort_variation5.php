<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays containing different string data to rsort() to test behaviour
 */

echo "*** Testing rsort() : variation ***\n";

$various_arrays = array (
// group of escape sequences
array(null, NULL, "\a", "\cx", "\e", "\f", "\n", "\t", "\xhh", "\ddd", "\v"),

// array contains combination of capital/small letters
array("lemoN", "Orange", "banana", "apple", "Test", "TTTT", "ttt", "ww", "x", "X", "oraNGe", "BANANA")
);

$flags = array("SORT_REGULAR" => SORT_REGULAR, "SORT_STRING" => SORT_STRING);

$count = 1;
// loop through to test rsort() with different arrays
foreach ($various_arrays as $array) {
	echo "\n-- Iteration $count --\n";

	echo "- With Default sort flag -\n";
	$temp_array = $array;
	var_dump(rsort($temp_array) );
	var_dump($temp_array);

	// loop through $flags array and setting all possible flag values
	foreach($flags as $key => $flag){
		echo "- Sort flag = $key -\n";
		
		$temp_array = $array;
		var_dump(rsort($temp_array, $flag) );
		var_dump($temp_array);
	}
	$count++;
}

echo "Done";
?>
