<?php 

$aDOM = new DOMDocument();
$aDOM->appendChild($aDOM->createElementNS('http://friend2friend.net/','f2f:a'));

$fromdom = new DOMDocument();
$fromdom->loadXML('<data xmlns:ai="http://altruists.org" ai:attr="namespaced" />');

$attr= $fromdom->firstChild->attributes->item(0);

$att = $aDOM->importNode($attr);

$aDOM->documentElement->appendChild($aDOM->importNode($attr, true));

echo $aDOM->saveXML();

?>
