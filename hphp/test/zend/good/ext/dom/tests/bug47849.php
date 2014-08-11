<?php 

$aDOM= new DOMDocument();
$aDOM->appendChild($aDOM->createElementNS('urn::root','r:root'));

$fromdom= new DOMDocument();
$fromdom->loadXML('<data xmlns="urn::data">aaa</data>');

$data= $fromdom->documentElement;
$aDOM->documentElement->appendChild($aDOM->importNode($data));

echo $aDOM->saveXML();

?>
