<?php
ini_set('precision', 14);

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

$iterator = 1;
foreach($values as $value) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(exp($value));
	$iterator++;
};

?>
===Done===