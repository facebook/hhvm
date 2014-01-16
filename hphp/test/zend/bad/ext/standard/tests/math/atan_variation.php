<?php
/* 
 * proto float atan(float number)
 * Function is implemented in ext/standard/math.c
*/ 


//Test atan with a different input values

$values = array(23,
		-23,
		2.345e1,
		-2.345e1,
		0x17,
		027,
		"23",
		"23.45",
		"2.345e1",
		"nonsense",				
		"1000",
		"1000ABC",
		null,
		true,
		false);	

for ($i = 0; $i < count($values); $i++) {
	$res = atan($values[$i]);
	var_dump($res);
}

?>