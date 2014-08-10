<?php
/* Prototype  : bool ctype_alnum(mixed $c)
 * Description: Checks for alphanumeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

echo "*** Testing ctype_alnum() : basic functionality ***\n";

$orig = setlocale(LC_CTYPE, "C"); 

$c1 = 'abcXYZ';
$c2 = ' \t*@';

var_dump(ctype_alnum($c1));
var_dump(ctype_alnum($c2));

setlocale(LC_CTYPE, $orig);
?>
===DONE===
