<?hh
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

	ZendGoodExtSoapTestsServer024::$server->addSoapHeader(new TestHeader1("Hello Header!"));
	ZendGoodExtSoapTestsServer024::$server->addSoapHeader(new TestHeader2("Hello Header!"));
	return "Hello Body!";
}

ZendGoodExtSoapTestsServer024::$server = new soapserver(null,darray['uri'=>"http://testuri.org"]);
ZendGoodExtSoapTestsServer024::$server->addfunction("test");

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:si="http://soapinterop.org/xsd">
  <SOAP-ENV:Body>
    <ns1:test xmlns:ns1="http://testuri.org"/>
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;

ZendGoodExtSoapTestsServer024::$server->handle($HTTP_RAW_POST_DATA);
echo "ok\n";

abstract final class ZendGoodExtSoapTestsServer024 {
  public static $server;
}
