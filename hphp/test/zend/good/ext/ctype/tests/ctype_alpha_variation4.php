<?php
/* Prototype  : bool ctype_alpha(mixed $c)
 * Description: Checks for alphabetic character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass octal and hexadecimal values to ctype_alpha() to test behaviour
 */

echo "*** Testing ctype_alpha() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

$octal_values = array (0101, 0102, 0103, 0104);
$hex_values = array   (0x41, 0x42, 0x43, 0x44);

echo "\n-- Octal Values --\n";
$iterator = 1;
foreach($octal_values as $c) {
	echo "-- Iteration $iterator --\n";
	var_dump(ctype_alpha($c));
	$iterator++;
}

echo "\n-- Hexadecimal Values --\n";
$iterator = 1;
foreach($hex_values as $c) {
	echo "-- Iteration $iterator --\n";
	var_dump(ctype_alpha($c));
	$iterator++;
}

setlocale(LC_CTYPE, $orig);
?>
===DONE===