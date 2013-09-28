<?php
/* Prototype  :  string number_format  ( float $number  [, int $decimals  ] )
 *               string number_format ( float $number , int $decimals , string $dec_point , string $thousands_sep )
 * Description: Format a number with grouped thousands
 * Source code: ext/standard/string.c
 */
 
echo "*** Testing number_format() : basic functionality ***\n";

$values = array(1234.5678,
				-1234.5678,
				1234.6578e4,
				-1234.56789e4,
				0x1234CDEF,
				02777777777,
				"123456789",
				"123.456789",
				"12.3456789e1",				
				null,
				true,
				false);	

echo "\n-- number_format tests.....default --\n";
for ($i = 0; $i < count($values); $i++) {
	$res = number_format($values[$i]);
	var_dump($res);
}

echo "\n-- number_format tests.....with two dp --\n";
for ($i = 0; $i < count($values); $i++) {
	$res = number_format($values[$i], 2);
	var_dump($res);
}

echo "\n-- number_format tests.....English format --\n";
for ($i = 0; $i < count($values); $i++) {
	$res = number_format($values[$i], 2, '.', ' ');
	var_dump($res);
}

echo "\n-- number_format tests.....French format --\n";
for ($i = 0; $i < count($values); $i++) {
	$res = number_format($values[$i], 2, ',' , ' ');
	var_dump($res);
}
?>
===DONE===