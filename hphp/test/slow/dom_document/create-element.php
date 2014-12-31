<?php

class SampleElement extends DOMElement {}
$dom = new DOMDocument();
$dom->registerNodeClass('DOMElement', 'SampleElement');

var_dump(get_class($dom->createElement('test')));
var_dump(get_class($dom->createElementNS('n:s', 'test')));
