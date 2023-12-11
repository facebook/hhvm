<?hh
class TestSoapClient extends SoapClient{
  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
		return <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://schemas.nothing.com" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body>
<ns1:dotest2Response><res xsi:type="ns1:book">
  <a xsi:type="xsd:string">Blaat</a>
  <b xsi:type="xsd:string">aap</b>
</res>
</ns1:dotest2Response></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;
	}
}

class book{
	public $a="a";
	public $b="c";

}
<<__EntryPoint>>
function main_entry(): void {

  $options=dict[
  		'actor' =>'http://schema.nothing.com',
  		'classmap' => dict['book'=>'book', 'wsdltype2'=>'classname2']
  		];

  $client = new TestSoapClient(dirname(__FILE__)."/classmap.wsdl",$options);
  $ret = $client->__soapcall('dotest2', vec["???"]);
  var_dump($ret);
  echo "ok\n";
}
