<?php
/* Prototype  : bool ctype_punct(mixed $c)
 * Description: Checks for any printable character which is not whitespace 
 * or an alphanumeric character 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_punct() to test which character codes are considered
 * valid punctuation characters
 */

echo "*** Testing ctype_punct() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C"); 

for ($c = 1; $c < 256; $c++) {
	if (ctype_punct($c)) {
		echo "character code $c is punctuation\n";
	}
}

setlocale(LC_CTYPE, $orig); 
?>
===DONE===