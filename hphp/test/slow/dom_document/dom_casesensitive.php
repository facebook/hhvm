<?php

$doc = new DOMDocument();
$doc->loadXML('<x:x xmlns:x="urn:x"/>');
$node = $doc->documentElement;

var_dump($node->nodeName);
var_dump($node->namespaceURI);
var_dump($node->prefix);
var_dump($node->localName);
var_dump($node->textContent);

var_dump($node->nodename);
var_dump($node->namespaceuri);
var_dump($node->PREFIX);
var_dump($node->localname);
var_dump($node->textcontent);
