<?php

$doc = new DOMDocument('1.0');

$root = $doc->createElement('html');
$doc->appendChild($root);

$head = $doc->createElement('head');
$root->appendChild($head);

$title = $doc->createElement('title');
$head->appendChild($title);

$text = $doc->createTextNode('Unit Test');
$title->appendChild($text);

$body = $doc->createElement('body');
$root->appendChild($body);

var_dump($doc->saveXML());
var_dump($doc->saveXML($head));
