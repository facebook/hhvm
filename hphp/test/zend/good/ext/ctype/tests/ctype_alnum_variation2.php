<?php
/* Prototype  : bool ctype_alnum(mixed $c)
 * Description: Checks for alphanumeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_alnum() to test which character codes are considered
 * valid alphanumeric characters
 */

echo "*** Testing ctype_alnum() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C"); 

for ($i = 0; $i < 256; $i++) {
	if (ctype_alnum($i)) {
		echo "character code $i is alpha numeric\n";
	}
}

setlocale(LC_CTYPE, $orig);
?>
===DONE===