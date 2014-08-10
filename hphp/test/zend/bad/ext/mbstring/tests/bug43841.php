<?php
/* Prototype  : int mb_strrpos(string $haystack, string $needle [, int $offset [, string $encoding]])
 * Description: Find position of last occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test that mb_strrpos offset is byte count for negative values (should be character count)
 */

$offsets = array(-25, -24, -13, -12);
$string_mb =
base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvv
JfvvJjvvJnjgII=');
$needle = base64_decode('44CC');

foreach ($offsets as $i) {
	echo "\n-- Offset is $i --\n";
	echo "Multibyte String:\n";
	var_dump( mb_strrpos($string_mb, $needle, $i, 'UTF-8') );
	echo "ASCII String:\n";
	echo "mb_strrpos:\n";
	var_dump(mb_strrpos(b'This is na English ta', b'a', $i));
	echo "strrpos:\n";
	var_dump(strrpos(b'This is na English ta', b'a', $i));
}
?>

