<?php
$dom = new DOMDocument();

$frag1 = $dom->createDocumentFragment();
var_dump($dom->saveHTML($frag1));

$frag2 = $dom->createDocumentFragment();
$div = $dom->createElement('div');
$div->appendChild($dom->createElement('span'));
$frag2->appendChild($div);
$frag2->appendChild($dom->createElement('div'));
$frag2->appendChild($dom->createElement('div'));
var_dump($dom->saveHTML($frag2));
?>
===DONE===
