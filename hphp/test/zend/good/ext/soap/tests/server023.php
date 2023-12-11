<?hh
function test() :mixed{

	ZendGoodExtSoapTestsServer023::$server->addSoapHeader(new SoapHeader("http://testuri.org", "Test1", "Hello Header!"));
	ZendGoodExtSoapTestsServer023::$server->addSoapHeader(new SoapHeader("http://testuri.org", "Test2", "Hello Header!"));
	return "Hello Body!";
}

abstract final class ZendGoodExtSoapTestsServer023 {
  public static $server;
}
<<__EntryPoint>>
function entrypoint_server023(): void {

  ZendGoodExtSoapTestsServer023::$server = new SoapServer(null,dict['uri'=>"http://testuri.org"]);
  ZendGoodExtSoapTestsServer023::$server->addFunction("test");

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

  ZendGoodExtSoapTestsServer023::$server->handle($HTTP_RAW_POST_DATA);
  echo "ok\n";
}
