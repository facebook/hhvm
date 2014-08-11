<?php
	$domDocument = new DOMDocument();
	$domDocument->substituteEntities = true;
	$domDocument->load(dirname(__FILE__) . DIRECTORY_SEPARATOR . "bug67081_0.xml");
	var_dump($domDocument->doctype->internalSubset);

	$domDocument = new DOMDocument();
	$domDocument->substituteEntities = true;
	$domDocument->load(dirname(__FILE__) . DIRECTORY_SEPARATOR . "bug67081_1.xml");
	var_dump($domDocument->doctype->internalSubset);

	$domDocument = new DOMDocument();
	$domDocument->substituteEntities = true;
	$domDocument->load(dirname(__FILE__) . DIRECTORY_SEPARATOR . "bug67081_2.xml");
	var_dump($domDocument->doctype->internalSubset);

	$domDocument = new DOMDocument();
	$domDocument->substituteEntities = true;
	$domDocument->load(dirname(__FILE__) . DIRECTORY_SEPARATOR . "dom.xml");
	var_dump($domDocument->doctype->internalSubset);
?>
===DONE===
