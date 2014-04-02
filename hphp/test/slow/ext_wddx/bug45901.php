<?php

$xml = new SimpleXMLElement('<data></data>');
$xml->addChild('test');
echo wddx_serialize_value($xml,'Variables') . "\n";
echo "DONE";
?>
