<?php
	$objDoc = new DomDocument();
	
	try { $objRef = $objDoc->createEntityReference(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
?>
===DONE===
