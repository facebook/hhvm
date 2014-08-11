<?php

$doc = new DOMDocument;
$doc->load(dirname(__FILE__)."/book.xml");

var_dump($doc->preserveWhiteSpace);

$doc->preserveWhiteSpace = false;
var_dump($doc->preserveWhiteSpace);

?>
