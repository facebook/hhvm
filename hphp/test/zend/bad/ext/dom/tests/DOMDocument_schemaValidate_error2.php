<?php

$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book.xml");

$result = $doc->schemaValidate(dirname(__FILE__)."/book-non-conforming-schema.xsd");
var_dump($result);

?>
