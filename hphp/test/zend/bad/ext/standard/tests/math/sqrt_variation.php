<?php
ini_set('precision ',  14);

/* 
 * proto float sqrt(float number)
 * Function is implemented in ext/standard/math.c
*/ 


//Test sqrt with a different input values
echo "*** Testing sqrt() : usage variations ***\n";

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
	$res = sqrt($values[$i]);
	var_dump($res);
}

?>
===Done===