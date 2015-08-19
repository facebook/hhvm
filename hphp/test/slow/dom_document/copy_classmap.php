<?php
class SampleElement extends \DOMElement {}

$dom = new DOMDocument();
$dom->registerNodeClass('DOMElement', 'SampleElement');
$dom->loadXML('<?xml version="1.0" ?><root />');

$list = $dom->childNodes;

var_dump(get_class($list->item(0)));
