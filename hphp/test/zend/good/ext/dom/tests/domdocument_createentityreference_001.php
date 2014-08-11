<?php
	$objDoc = new DomDocument();
	
	$objRef = $objDoc->createEntityReference('Test');
	echo $objRef->nodeName . "\n";
?>
===DONE===
