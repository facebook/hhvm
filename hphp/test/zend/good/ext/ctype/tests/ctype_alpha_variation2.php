<?php
/* Prototype  : bool ctype_alpha(mixed $c)
 * Description: Checks for alphabetic character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_alpha() to test which character codes are considered
 * valid alphabetic characters
 */

echo "*** Testing ctype_alpha() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

for ($i = 0; $i < 256; $i++) {
	if (ctype_alpha($i)) {
		echo "character code $i is alphabetic\n";
	}
}

setlocale(LC_CTYPE, $orig);
?>
===DONE===