<?php
ini_set('precision', 14);

/* Prototype  : float expm1 ( float $arg  )
 * Description: Returns exp(number) - 1, computed in a way that is accurate even 
 *              when the value of number is close to zero.
 * Source code: ext/standard/math.c
 */

echo "*** Testing expm1() : basic functionality ***\n";
$values = array(10,
				10.3,
				3.9505e3,
				037,
				0x5F,	
				"10",
				"3950.5",
				"3.9505e3",
				"039",
				"0x5F",
				true,
				false,
				null, 
				);	

// loop through each element of $values to check the behaviour of expm1()
$iterator = 1;
foreach($values as $value) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(expm1($value));
	$iterator++;
};
?>
===Done===