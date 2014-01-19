<?php
/* Prototype  : bool ctype_graph(mixed $c)
 * Description: Checks for any printable character(s) except space 
 * Source code: ext/ctype/ctype.c 
 */

/*
 * Pass different integers to ctype_graph() to test which character codes are considered
 * valid visibly printable characters
 */

echo "*** Testing ctype_graph() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

for ($i = 0; $i < 256; $i++) {
	if (ctype_graph($i)) {
		echo "character code $i is a printable character\n";
	}
}

setlocale(LC_CTYPE, $orig);
?>
===DONE===