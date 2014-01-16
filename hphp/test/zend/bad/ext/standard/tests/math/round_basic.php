<?php
ini_set('precision', 14);

/* Prototype  : float round  ( float $val  [, int $precision  ] )
 * Description: Returns the rounded value of val  to specified precision (number of digits
 * after the decimal point)
 * Source code: ext/standard/math.c
 */

echo "*** Testing round() : basic functionality ***\n";

$values = array(123456789,
				123.456789,
				-4.5679123,
				1.23E4,
				-4.567E3,
				0x234567,
				067777777,
				"1.234567", 
				"2.3456789e8",
				"0x1234CDEF");			
					
$precision = array(2,
				8,
				0x3,
				04,
				3.6,
				"2",
				"0x03",
				"04",
				"3.6",
				"2.1e1",				
				null,
				true,
				false);					

for ($i = 0; $i < count($values); $i++) {
	echo "round: $values[$i]\n";
	for ($j = 0; $j < count($precision); $j++) {
		$res = round($values[$i], $precision[$j]);
		echo "...with precision $precision[$j]-> ";
		var_dump($res);
	}	
}
?>
===Done===