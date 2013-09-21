<?php

/* Prototype  : int ord  ( string $string  )
 * Description: Return ASCII value of character
 * Source code: ext/standard/string.c
*/

echo "*** Testing ord() : basic functionality ***\n";

var_dump(ord("a"));
var_dump(ord("z"));
var_dump(ord("0"));
var_dump(ord("9"));
var_dump(ord("!"));
var_dump(ord("*"));
var_dump(ord("@"));
var_dump(ord("\n"));
var_dump(ord("\x0A"));
var_dump(ord("\xFF"));
var_dump(ord("Hello"));

// Make sure all valid ascii chars round trip
for ($i = 0; $i < 255; $i++) {
	if (ord(chr($i)) != $i) {
		exit("TEST FAILED: $i does not round trip\n");
	} 	
}

?>
===DONE===