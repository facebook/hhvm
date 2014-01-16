<?php
ini_set('precision', 14);

/* Prototype  : float ceil  ( float $value  )
 * Description: Round fractions up.
 * Source code: ext/standard/math.c
 */

echo "*** Testing ceil() : basic functionality ***\n";
$values = array(0,
				-0,
				0.5,
				-0.5,
				1,
				-1,
				1.5,
				-1.5,
				2.6,
				-2.6,
				037,
				0x5F,	
				"10.5",
				"-10.5",
				"3.95E3",
				"-3.95E3",
				"039",
				"0x5F",
				true,
				false,
				null, 
				);	

for ($i = 0; $i < count($values); $i++) {
	$res = ceil($values[$i]);
	var_dump($res);
}

?>
===Done===