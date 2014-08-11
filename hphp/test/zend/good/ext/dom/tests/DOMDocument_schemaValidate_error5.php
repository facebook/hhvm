<?php

$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book.xml");

$result = $doc->schemaValidate(dirname(__FILE__)."/non-existent-file");
var_dump($result);

?>
