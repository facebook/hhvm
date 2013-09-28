<?php
/* Prototype  : bool ctype_upper(mixed $c)
 * Description: Checks for uppercase character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_upper() to test which character codes are considered
 * valid uppercase characters
 */

echo "*** Testing ctype_upper() : usage variations ***\n";
$orig = setlocale(LC_CTYPE, "C");

for ($i = 0; $i < 256; $i++) {
	if (ctype_upper($i)) {
		echo "character code $i is a uppercase character\n";
	}
}

setlocale(LC_CTYPE, $orig);
?>
===DONE===