<?php 
$root = simplexml_load_string('<?xml version="1.0"?>
<root>
 <child attribute="Sample" />
</root>
');

echo $root->child['attribute'];
echo "\n---Done---\n";
?>