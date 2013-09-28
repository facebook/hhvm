<?php

$request = "<?xml version=\"1.0\"?>\n<xml have=\"attributes\" />\n";
$xml = new SimpleXMLElement($request);
var_dump(!$xml);
