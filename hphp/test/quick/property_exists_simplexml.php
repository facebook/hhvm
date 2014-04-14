<?php
$xml = new SimpleXmlElement('<root/>');
$xml->addChild('a', '1');

var_dump(property_exists($xml, 'a'));
