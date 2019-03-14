<?php
class TestHeader1 extends SoapHeader {
	function __construct($data) {
		parent::__construct("http://testuri.org", "Test1", $data);
	}
}

class TestHeader2 extends SoapHeader {
	function __construct($data) {
		parent::__construct("http://testuri.org", "Test2", $data);
	}
}

function test() {

	ZendGoodExtSoapTestsServer025::$server->addSoapHeader(new TestHeader1("Hello Header!"));
	ZendGoodExtSoapTestsServer025::$server->addSoapHeader(new TestHeader2("Hello Header!"));
	return "Hello Body!";
}

ZendGoodExtSoapTestsServer025::$server = new soapserver(dirname(__FILE__)."/server025.wsdl");
ZendGoodExtSoapTestsServer025::$server->addfunction("test");

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/">
  <SOAP-ENV:Body>
    <ns1:test xmlns:ns1="http://testuri.org"/>
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;

ZendGoodExtSoapTestsServer025::$server->handle($HTTP_RAW_POST_DATA);
echo "ok\n";

abstract final class ZendGoodExtSoapTestsServer025 {
  public static $server;
}
