<?php

function add($a, $b) {
 return $a + $b;
 }
$server = new SoapServer(NULL, array('uri' => 'http://test-uri'));
$str = '<?xml version="1.0" '.       'encoding="ISO-8859-1"?>'.       '<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='.       '"http://schemas.xmlsoap.org/soap/encoding/"'.       ' xmlns:SOAP-ENV='.       '"http://schemas.xmlsoap.org/soap/envelope/"'.       ' xmlns:xsd="http://www.w3.org/2001/XMLSchema"'.       ' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'.       ' xmlns:si="http://soapinterop.org/xsd"><SOAP-ENV:Body>'.       '<ns1:Add xmlns:ns1="http://testuri.org">'.       '<x xsi:type="xsd:hexBinary">16</x>'.       '<y xsi:type="xsd:hexBinary">21</y>'.       '</ns1:Add>  </SOAP-ENV:Body></SOAP-ENV:Envelope>';
$server->addFunction('Add');
$server->handle($str);
