<?php
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays of string data to see how natcasesort() re-orders the array
 */

echo "*** Testing natcasesort() : usage variation ***\n";

$inputs = array (
	// group of escape sequences
	array(null, NULL, "\a", "\cx", "\e", "\f", "\n", "\t", "\xhh", "\ddd", "\v"),

	// array contains combination of capital/small letters
	array("lemoN", "Orange", "banana", "apple", "Test", "TTTT", "ttt", "ww", "x", "X", "oraNGe", "BANANA")
);

foreach ($inputs as $array_arg) {
	var_dump( natcasesort($array_arg) );
	var_dump($array_arg);
}

echo "Done";
?>