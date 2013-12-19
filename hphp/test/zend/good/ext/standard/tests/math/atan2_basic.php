<?php
$valuesy = array(23,
				-23,
				2.345e1,
				-2.345e1,
				0x17,
				027,
				"23",
				"23.45",
				"2.345e1",				
				null,
				true,
				false);
					
$valuesx = array(23,
				-23,
				2.345e1,
				-2.345e1,
				0x17,
				027,
				"23",
				"23.45",
				"2.345e1",				
				null,
				true,
				false);					

for ($i = 0; $i < count($valuesy); $i++) {
	for ($j = 0; $j < count($valuesx); $j++) {	
		$res = atan2($valuesy[$i], $valuesx[$j]);
		echo "Y:$valuesy[$i] X:$valuesx[$j] ";
		var_dump($res);
	}	
}
?>