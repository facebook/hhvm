<?php
ini_set('precision', 14);


/* Prototype  : float hypot  ( float $x  , float $y  )
 * Description: Calculate the length of the hypotenuse of a right-angle triangle.
 * Source code: ext/standard/math.c
 */

echo "*** Testing hypot() : basic functionality ***\n";

$valuesy = array(23,
				-23,
				2.345e1,
				-2.345e1,
				0x17,
				027,
				"23",
				"23.45",
				"2.345e1",	
				"23abc",			
				null,
				true,
				false);
					
$valuesx = array(33,
				-33,
				3.345e1,
				-3.345e1,
				0x27,
				037,
				"33",
				"43.45",
				"1.345e1",
				"33abc",				
				null,
				true,
				false);					

for ($i = 0; $i < count($valuesy); $i++) {
	for ($j = 0; $j < count($valuesx); $j++) {	
		echo "\nY:$valuesy[$i] X:$valuesx[$j] ";
		$res = hypot($valuesy[$i], $valuesx[$j]);
		var_dump($res);
	}	
}
?>
===Done===