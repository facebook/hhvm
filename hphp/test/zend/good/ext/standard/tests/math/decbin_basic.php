<?php
$values = array(10,
				3950.5,
				3.9505e3,
				039,
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

for ($i = 0; $i < count($values); $i++) {
	$res = decbin($values[$i]);
	var_dump($res);
}
?>
