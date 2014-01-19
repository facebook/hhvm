<?php

$xml = '<?xml version="1.0" encoding="utf-8" ?>
<test>

</test>';

$root = simplexml_load_string($xml);
$clone = clone $root;
var_dump($root);
var_dump($clone);
var_dump($clone->asXML());

echo "Done\n";
?>