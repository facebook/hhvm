<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://test.pl"><SOAP-ENV:Body>
<SOAP-ENV:Test>
<parameters priority="high">
<ns1:NetworkErrorCode>1</ns1:NetworkErrorCode>
</parameters>
</SOAP-ENV:Test></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;

function Test($x) {
  return $x->priority;
}

$x = new SoapServer(dirname(__FILE__)."/bug39832.wsdl");
$x->addFunction("Test");
$x->handle($HTTP_RAW_POST_DATA);
?>