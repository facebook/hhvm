<?php
class Foo {
  function __call($name, $args) {
    if ($name == "test") {
      return "Hello World";
    } else {
      return SoapFault("Server","Function $name doesn't exist");
    }
  }
}

$foo = new Foo();
$server = new SoapServer(null,array('uri'=>"http://testuri.org"));
$server->setObject($foo);

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:si="http://soapinterop.org/xsd">
  <SOAP-ENV:Body>
    <ns1:test xmlns:ns1="http://testuri.org" />
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;

$server->handle($HTTP_RAW_POST_DATA);
echo "ok\n";
?>