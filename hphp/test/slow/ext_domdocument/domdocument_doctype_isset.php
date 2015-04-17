<?php
$dom = new DOMDocument();
var_dump(isset($dom->doctype));
var_dump($dom->doctype);

$implementation = new DOMImplementation();
$dtd = $implementation->createDocumentType('graph', '', 'graph.dtd');

$dom = $implementation->createDocument('', '', $dtd);
var_dump(isset($dom->doctype));
var_dump($dom->doctype);
