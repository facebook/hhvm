<?php

$root = simplexml_load_string(b'<?xml version="1.0"?>
<root>
 <child>Hello</child>
 <child>World</child>
</root>
');

foreach (new IteratorIterator($root->child) as $child) {
	echo $child."\n";
}
?>
===DONE===
<?php exit(0); ?>