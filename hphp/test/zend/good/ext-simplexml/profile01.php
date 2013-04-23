<?php 
$root = simplexml_load_string('<?xml version="1.0"?>
<root>
 <child>Hello</child>
</root>
');

echo $root->child;
echo "\n---Done---\n";
?>