<?php

$doc1 = new SimpleXMLElement('<test><a>content</a></test>');
var_dump($doc1->xpath("a"));

$doc2 = clone $doc1;
$doc1 = null; // verify doc2 does not depend on doc1
var_dump($doc2->xpath("a"));
