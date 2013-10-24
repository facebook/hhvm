<?php

$doc = new DOMDocument;

$node1 = $doc->createElement('div');
$node2 = $doc->createElement('div');

$res1 = $doc->appendChild($node1);
$res2 = $doc->insertBefore($node2, $res1);
$res3 = $doc->insertBefore($node1, $node2);

var_dump($doc->saveXML());
