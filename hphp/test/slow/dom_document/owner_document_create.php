<?php
$dom = new DOMDocument();
var_dump($dom->createComment('Foo')->ownerDocument instanceof DOMDocument);
var_dump($dom->importNode($dom->createComment('Foo'))->ownerDocument instanceof DOMDocument);
