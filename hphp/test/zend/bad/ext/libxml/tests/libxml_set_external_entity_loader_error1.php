<?php
$xml = <<<XML
<!DOCTYPE foo PUBLIC "-//FOO/BAR" "http://example.com/foobar">
<foo>bar</foo>
XML;

$dd = new DOMDocument;
$r = $dd->loadXML($xml);

var_dump(libxml_set_external_entity_loader([]));
var_dump(libxml_set_external_entity_loader());
var_dump(libxml_set_external_entity_loader(function() {}, 2));

var_dump(libxml_set_external_entity_loader(function($a, $b, $c, $d) {}));
var_dump($dd->validate());

echo "Done.\n";
