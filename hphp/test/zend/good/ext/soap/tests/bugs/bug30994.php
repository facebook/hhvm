<?hh

function bassCall() :mixed{
  return "ok";
}
<<__EntryPoint>>
function entrypoint_bug30994(): void {
  $HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/"
	xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"
	xmlns:tns="http://spock/kunta/kunta"
	xmlns:types="http://spock/kunta/kunta/encodedTypes"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xmlns:xsd="http://www.w3.org/2001/XMLSchema">

<soap:Body
soap:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
	<q1:bassCall xmlns:q1="http://spock/bass/types/kunta">
		<system xsi:type="xsd:string">XXX</system>
		<function xsi:type="xsd:string">TASKTEST</function>
		<parameter href="#id1" />
	</q1:bassCall>

	<soapenc:Array id="id1" soapenc:arrayType="tns:Item[1]">
		<Item href="#id2" />
	</soapenc:Array>

	<tns:Item id="id2" xsi:type="tns:Item">
		<key xsi:type="xsd:string">ABCabc123</key>
		<val xsi:type="xsd:string">123456</val>
	</tns:Item>

</soap:Body>
</soap:Envelope>
EOF;

  $x = new SoapServer(NULL, dict["uri"=>"http://spock/kunta/kunta"]);
  $x->addFunction("bassCall");
  $x->handle($HTTP_RAW_POST_DATA);
}
