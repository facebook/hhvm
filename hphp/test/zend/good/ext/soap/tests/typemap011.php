<?hh
class TestSoapClient extends SoapClient{
  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
		return <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://schemas.nothing.com" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body>
<ns1:dotest2Response><res xsi:type="ns1:book">
  <a xsi:type="xsd:string">foo</a>
  <b xsi:type="xsd:string">bar</b>
</res>
</ns1:dotest2Response></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;
	}
}

class book{
	public $a="a";
	public $b="c";

}

function book_from_xml($xml) :mixed{
	throw new SoapFault("Client", "Conversion Error");
}
<<__EntryPoint>>
function main_entry(): void {

  $options=dict[
  		'actor' =>'http://schemas.nothing.com',
  		'typemap' => vec[dict["type_ns"   => "http://schemas.nothing.com",
  		                         "type_name" => "book",
  		                         "from_xml"  => book_from_xml<>]]
  		];

  $client = new TestSoapClient(dirname(__FILE__)."/classmap.wsdl",$options);
  try {
  	$ret = $client->__soapcall('dotest2', vec["???"]);
  } catch (SoapFault $e) {
  	$ret = "SoapFault = " . $e->faultcode . " - " . $e->faultstring;
  }
  var_dump($ret);
  echo "ok\n";
}
