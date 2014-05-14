<?php

$element = new DOMElement("element", "somevalue");
$result = serialize($element);
print $result . "\n";
