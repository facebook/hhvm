<?php
/* Prototype  : bool ctype_xdigit(mixed $c)
 * Description: Checks for character(s) representing a hexadecimal digit 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_xdigit() to test which character codes are considered
 * valid hexadecimal 'digits'
 */

echo "*** Testing ctype_xdigit() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C"); 

for($c = 1; $c < 256; $c++) {
	if (ctype_xdigit($c)) {
		echo "character code $c is a hexadecimal 'digit'\n";
	}
}

setlocale(LC_CTYPE, $orig); 
?>
===DONE===