<?php
ini_set('precision', 14);

$bases = array(23,
				-23,
				23.1,
				-23.1,
				2.345e1,
				-2.345e1,
				0x17,
				027,
				"23",
				"23.45",
				"2.345e1",	
				PHP_INT_MAX,
				-PHP_INT_MAX - 1);			

$exponents = array(0,
               1,
               -1,
               2,
               -2,
               3,
               -3,
               2.5,
               -2.5,
               500,
               -500,
               2147483647,
			   -2147483648); 				
					
foreach($bases as $base) {
	echo "\n\nBase = $base";
	foreach($exponents as $exponent) {
		echo "\n..... Exponent = $exponent Result = ";
		$res = pow($base, $exponent);
		echo $res;
	}
	echo "\n\n";
}
?>
===Done===