<?hh
class foo {
  private $sum = 0;

  function Sum($num) :mixed{
    return $this->sum += $num;
  }
}
<<__EntryPoint>> function main(): void {
$server = new SoapServer(null,dict['uri'=>"http://testuri.org"]);
$server->setClass("foo");
$server->setpersistence(SOAP_PERSISTENCE_REQUEST);


ob_start();
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:si="http://soapinterop.org/xsd">
  <SOAP-ENV:Body>
    <ns1:Sum xmlns:ns1="http://testuri.org">
      <num xsi:type="xsd:int">5</num>
    </ns1:Sum>
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;
$server->handle($HTTP_RAW_POST_DATA);
ob_clean();

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:si="http://soapinterop.org/xsd">
  <SOAP-ENV:Body>
    <ns1:Sum xmlns:ns1="http://testuri.org">
      <num xsi:type="xsd:int">3</num>
    </ns1:Sum>
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;
$server->handle($HTTP_RAW_POST_DATA);
ob_end_flush();
echo "ok\n";
}
