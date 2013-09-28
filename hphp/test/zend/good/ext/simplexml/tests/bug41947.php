<?php
$xml = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><root xmlns:myns="http://myns" />');
$grandchild = $xml->addChild('child', null, 'http://myns')->addChild('grandchild', 'hello', '');

$gchild = $xml->xpath("//grandchild");
if (count($gchild) > 0) {
    echo $gchild[0]."\n";
}
?>
===DONE===