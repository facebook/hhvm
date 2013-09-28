<?php

$xml =<<<EOF
<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope
xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/" 
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
xmlns:xsd="http://www.w3.org/2001/XMLSchema"
>
<soap:Body>
<businessList foo="bar">
<businessInfo businessKey="bla"/>
</businessList>
</soap:Body> 
</soap:Envelope>
EOF;

$sxe = simplexml_load_string($xml);
$nsl = $sxe->getNamespaces();
var_dump($nsl);

$sxe = simplexml_load_string($xml, NULL, 0, $nsl['soap']);
var_dump($sxe->Body);
var_dump($sxe->Body->children(''));
var_dump($sxe->Body->children('')->businessList);

?>
===DONE===