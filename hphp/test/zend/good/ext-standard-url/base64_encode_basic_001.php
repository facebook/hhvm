<?php
/* Prototype  : proto string base64_encode(string str)
 * Description: Encodes string using MIME base64 algorithm 
 * Source code: ext/standard/base64.c
 * Alias to functions: 
 */

/*
 * Test base64_encode with single byte values.
 */

echo "*** Testing base64_encode() : basic functionality ***\n";

for ($i=0; $i<256; $i++) {
	$str = pack("c", $i);
	$enc = base64_encode($str);
	printf("0x%X: %s\n", $i, $enc);
}

echo "Done";
?>