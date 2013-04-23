<?php 
$root = simplexml_load_string('<?xml version="1.0"?>
<root>
 <child>Hello</child>
 <child>World</child>
</root>
');

foreach ($root->child as $child) {
	echo "$child ";
}
echo "\n---Done---\n";
?>