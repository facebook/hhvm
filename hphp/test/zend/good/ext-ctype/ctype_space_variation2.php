<?php
/* Prototype  : bool ctype_space(mixed $c)
 * Description: Checks for whitespace character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_space() to test which character codes are considered
 * valid whitespace characters
 */

echo "*** Testing ctype_space() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

for ($c = 1; $c < 256; $c++) {
	if (ctype_space($c)) {
		echo "character code $c is a space character\n";
	}
}
setlocale(LC_CTYPE, $orig);
?>
===DONE===