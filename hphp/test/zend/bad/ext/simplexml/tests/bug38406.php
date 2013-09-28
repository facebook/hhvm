<?php

$item = new SimpleXMLElement(b'<something />');
$item->attribute = b'something';
var_dump($item->attribute);

$item->otherAttribute = $item->attribute;
var_dump($item->otherAttribute);

$a = array();
$item->$a = new stdclass;

echo "Done\n";
?>