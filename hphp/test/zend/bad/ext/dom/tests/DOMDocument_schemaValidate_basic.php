<?php

$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book.xml");

$result = $doc->schemaValidate(dirname(__FILE__)."/book.xsd");
var_dump($result);

?>
