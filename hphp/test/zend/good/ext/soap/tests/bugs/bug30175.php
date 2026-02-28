<?hh

class LocalSoapClient extends SoapClient {

  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
    return <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope
xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xmlns:xsd="http://www.w3.org/2001/XMLSchema"
xmlns:ns1="urn:qweb">
<SOAP-ENV:Body
SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
id="_0">
<ns1:HostInfo xsi:type="ns1:HostInfo">
<name xsi:type="xsd:string">blah blah some name field</name>
<shortDescription xsi:type="xsd:string">This is a description. more blah blah blah</shortDescription>
<ipAddress xsi:type="xsd:string">127.0.0.1</ipAddress>
</ns1:HostInfo>
</SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;
  }

}
<<__EntryPoint>>
function main_entry(): void {

  $client = new LocalSoapClient(dirname(__FILE__)."/bug30175.wsdl");
  var_dump($client->__soapcall('qwebGetHostInfo', vec[]));
}
