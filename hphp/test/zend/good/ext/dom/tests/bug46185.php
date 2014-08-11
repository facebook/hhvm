<?php 
$aDOM = new DOMDocument();
$aDOM->loadXML('<?xml version="1.0"?>
<ns1:a xmlns:ns1="urn::ns"/>');
$a= $aDOM->firstChild;

$ok = new DOMDocument();
$ok->loadXML('<?xml version="1.0"?>
<ns1:ok xmlns:ns1="urn::ns" xmlns="urn::REAL"><watch-me xmlns:default="urn::BOGUS"/></ns1:ok>');

$imported= $aDOM->importNode($ok->firstChild, true);
$a->appendChild($imported);

echo $aDOM->saveXML();
?>
