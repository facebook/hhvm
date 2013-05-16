<?php
	// Create a fixed array
	$fixedArray = new SplFixedArray(5);
	
	// Fill it up
	for ($i=0; $i < 5; $i++) { 
		$fixedArray[$i] = "PHPNW Testfest";
	}
	
	// remove an item
	$fixedArray->offsetUnset("4");
	
	var_dump($fixedArray);
	
?>