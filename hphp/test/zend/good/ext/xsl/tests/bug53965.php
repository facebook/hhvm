<?php

$base = 'file://' . dirname(__FILE__) . DIRECTORY_SEPARATOR . '53965';

$xml = new DOMDocument();
$xml->load($base . DIRECTORY_SEPARATOR . 'collection.xml');

$xsl = new DOMDocument();
$xsl->load($base . DIRECTORY_SEPARATOR . 'collection.xsl');

$proc = new XSLTProcessor;
$proc->importStyleSheet($xsl);

echo $proc->transformToXML($xml);
?>
