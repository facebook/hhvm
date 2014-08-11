<?php

$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book.xml");

$xsd = file_get_contents(dirname(__FILE__)."/book-non-conforming-schema.xsd");

$result = $doc->schemaValidateSource($xsd);
var_dump($result);

?>
