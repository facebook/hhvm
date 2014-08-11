<?php

$doc = new DOMDocument;
$doc->load(dirname(__FILE__)."/book.xml");

var_dump($doc->strictErrorChecking);

$doc->strictErrorChecking = false;
var_dump($doc->strictErrorChecking);

?>
