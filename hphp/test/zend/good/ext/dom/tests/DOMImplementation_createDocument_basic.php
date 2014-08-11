<?php
$x = new DOMImplementation();
$doc = $x->createDocument(null, 'html');
echo $doc->saveHTML();
?>
