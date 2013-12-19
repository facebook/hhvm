<?php
/* Prototype  : number abs  ( mixed $number  )
 * Description: Returns the absolute value of number.
 * Source code: ext/standard/math.c
 */

echo "*** Testing abs() : basic functionality ***\n";

$values = array(23,
				-23,
				2.345e1,
				-2.345e1,
				0x17,
				027,
				"23",
				"-23",
				"23.45",
				"2.345e1",		
				"-2.345e1",			
				null,
				true,
				false);	

for ($i = 0; $i < count($values); $i++) {
	$res = abs($values[$i]);
	var_dump($res);
}
?>
===Done===