<?php

$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book.xml");

$result = $doc->schemaValidateSource('string that is not a schema');
var_dump($result);

?>
