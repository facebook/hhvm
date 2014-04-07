<?php

$doc = new DOMDocument;
$element = $doc->createElement('root');
$xml = simplexml_import_dom($element);
var_dump($xml->asXML());
