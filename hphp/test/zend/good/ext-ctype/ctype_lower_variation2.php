<?php
/* Prototype  : bool ctype_lower(mixed $c)
 * Description: Checks for lowercase character(s)  
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_lower() to test which character codes are considered
 * valid lowercase characters
 */

echo "*** Testing ctype_lower() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

for ($i = 0; $i < 256; $i++) {
	if (ctype_lower($i)) {
		echo "character code $i is a lower case character\n";
	}
}
 
setlocale(LC_CTYPE, $orig);
?>
===DONE===