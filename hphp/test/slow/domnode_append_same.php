<?php

$doc = new DOMDocument;

$node1 = $doc->createElement('div');

$res1 = $doc->appendChild($node1);
$res2 = $doc->appendChild($res1);

#var_dump($res1);
#var_dump($res2);

var_dump($doc->saveXML());
