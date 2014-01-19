<?php
/* Prototype  : bool ctype_print(mixed $c)
 * Description: Checks for printable character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_print() to test which character codes are considered
 * valid printable characters
 */

echo "*** Testing ctype_print() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

for ($i = 0; $i < 256; $i++) {
	if (ctype_print($i)) {
		echo "character code $i is a printable character\n";
	}
}

setlocale(LC_CTYPE, $orig);
?>
===DONE===