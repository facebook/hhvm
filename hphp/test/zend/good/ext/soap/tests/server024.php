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

function test() :mixed{

	ZendGoodExtSoapTestsServer024::$server->addSoapHeader(new TestHeader1("Hello Header!"));
	ZendGoodExtSoapTestsServer024::$server->addSoapHeader(new TestHeader2("Hello Header!"));
	return "Hello Body!";
}

abstract final class ZendGoodExtSoapTestsServer024 {
  public static $server;
}
<<__EntryPoint>>
function entrypoint_server024(): void {

  ZendGoodExtSoapTestsServer024::$server = new SoapServer(null,dict['uri'=>"http://testuri.org"]);
  ZendGoodExtSoapTestsServer024::$server->addFunction("test");

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
}
