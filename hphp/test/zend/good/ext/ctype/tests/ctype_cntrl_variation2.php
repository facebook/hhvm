<?php
/* Prototype  : bool ctype_cntrl(mixed $c)
 * Description: Checks for control character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_cntrl() to test which character codes are considered
 * valid control characters
 */

echo "*** Testing ctype_cntrl() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

for ($i = 0; $i < 256; $i++) {
	if (ctype_cntrl($i)) {
		echo "character code $i is control character\n";
	}
}

setlocale(LC_CTYPE, $orig);
?>
===DONE===