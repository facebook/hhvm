<?php
/* Prototype  : int mb_ereg(string $pattern, string $string [, array $registers])
 * Description: Regular expression match for multibyte string 
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test how character classes match a multibyte string
 */

echo "*** Testing mb_ereg() : usage variations ***\n";

mb_regex_encoding('utf-8');

//contains japanese characters, ASCII digits and different, UTF-8 encoded digits
$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

$character_classes = array (b'[[:alnum:]]+', /*1*/
                            b'[[:alpha:]]+',
                            b'[[:ascii:]]+',
                            b'[[:blank:]]+',
                            b'[[:cntrl:]]+',/*5*/
                            b'[[:digit:]]+',
                            b'[[:graph:]]+',
                            b'[[:lower:]]+',
                            b'[[:print:]]+',
                            b'[[:punct:]]+', /*10*/
                            b'[[:space:]]+',
                            b'[[:upper:]]+',
                            b'[[:xdigit:]]+'); /*13*/

$iterator = 1;
foreach ($character_classes as $pattern) {
	if (is_array(@$regs)) {
		$regs = null;
	}
	echo "\n-- Iteration $iterator --\n";
	var_dump(mb_ereg($pattern, $string_mb, $regs));
	if ($regs) {
		base64_encode_var_dump($regs);
	}
	$iterator++;
}
/**
 * replicate a var dump of an array but outputted string values are base64 encoded
 *
 * @param array $regs
 */
function base64_encode_var_dump($regs) {
	if ($regs) {
		echo "array(" . count($regs) . ") {\n";
		foreach ($regs as $key => $value) {
			echo "  [$key]=>\n  ";
			if (is_string($value)) {
				var_dump(base64_encode($value));
			} else {
				var_dump($value);
			}
		}
		echo "}\n";
	} else {
		echo "NULL\n";
	}
}
echo "Done";

?>