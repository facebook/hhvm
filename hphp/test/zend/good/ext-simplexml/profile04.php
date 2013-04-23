<?php 
$root = simplexml_load_string('<?xml version="1.0"?>
<root xmlns:reserved="reserved-ns">
 <reserved:child>Hello</reserved:child>
</root>
');

echo $root->children('reserved-ns')->child;
echo "\n---Done---\n";
?>