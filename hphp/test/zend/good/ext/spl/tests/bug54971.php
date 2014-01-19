<?php

$source = <<<XML
<root>
<node>val1</node>
<node>val2</node>
</root>
XML;


$doc = new DOMDocument();
$doc->loadXML($source);

$xpath = new DOMXPath($doc);
$items = $xpath->query('//node');

print_r(array_map('get_class', iterator_to_array($items, false)));
print_r(array_map('get_class', iterator_to_array($items, true)));
?>