<?php
parse_str("<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:si="http://soapinterop.org/xsd">
  <SOAP-ENV:Body>
    <ns1:test xmlns:ns1="http://testuri.org" />
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);


echo "INPUT: \n";
echo file_get_contents("php://input") . "\n";
echo "\n\n-----------\n\n";

function test() {
  return "Hello World";
}

$server = new soapserver(null,array('uri'=>"http://testuri.org"));
$server->addfunction("test");
$server->handle();
echo "ok\n";
?>