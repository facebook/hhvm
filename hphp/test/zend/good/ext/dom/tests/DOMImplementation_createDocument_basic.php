<?php <<__EntryPoint>> function main() {
$x = new DOMImplementation();
$doc = $x->createDocument(null, 'html');
echo $doc->saveHTML();
}
