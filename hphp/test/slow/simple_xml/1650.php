<?php

$doc = new SimpleXMLElement('<?xml version="1.0"?><root><node><option>1</option></node></root>');
 $doc->node->option = 0;
 var_dump($doc->asXML());
