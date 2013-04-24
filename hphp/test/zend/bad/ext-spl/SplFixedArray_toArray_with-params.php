<?php
	// Create a fixed array
	$fixedArray = new SplFixedArray(5);
	
	// Fill it up
	for ($i=0; $i < 5; $i++) { 
		$fixedArray[$i] = "PHPNW Testfest";
	}
	
	// Test count() returns correct error when parameters are passed.
	$fixedArray->count(1);
?>