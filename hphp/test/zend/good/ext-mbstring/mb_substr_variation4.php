<?php
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test how mb_substr() behaves when passed a range of integers as $start argument
 */

echo "*** Testing mb_substr() : usage variations ***\n";

mb_internal_encoding('UTF-8');

$string_ascii = b'+Is an English string'; //21 chars

$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII='); //21 chars

/*
 * Loop through integers as multiples of ten for $offset argument
 * 60 is larger than *BYTE* count for $string_mb
 */
for ($i = -60; $i <= 60; $i += 10) {
	if (@$a || @$b) {
		$a = null;
		$b = null;
	}
	echo "\n**-- Offset is: $i --**\n";
	echo "-- ASCII String --\n";
	$a = mb_substr($string_ascii, $i, 4);
	var_dump(base64_encode($a));
	echo "--Multibyte String --\n";
	$b = mb_substr($string_mb, $i, 4, 'UTF-8');
	if (strlen($a) == mb_strlen($b, 'UTF-8')) { // should return same length
		var_dump(base64_encode($b));
	} else {
		echo "Difference in length of ASCII string and multibyte string\n";
	}
	
}

echo "Done";
?>