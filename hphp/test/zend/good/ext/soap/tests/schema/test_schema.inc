<?hh

class LocalSoapClient extends SoapClient {
  public static $input = null;

  function __construct($wsdl) {
    parent::__construct($wsdl);
    $this->server = new SoapServer($wsdl);
    $this->server->addFunction('test');
  }

  function __dorequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }
}

function test($input) {
  LocalSoapClient::$input = $input;
}

function test_schema($schema, $type, $param) {

$wsdl  = <<<EOF
<definitions name="InteropTest"
    xmlns:xsd="http://www.w3.org/2001/XMLSchema"
    xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"
    xmlns:tns="http://test-uri/"
    xmlns:soap="http://schemas.xmlsoap.org/wsdl/soap/"
    xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/"
    xmlns="http://schemas.xmlsoap.org/wsdl/"
    targetNamespace="http://test-uri/"
    >
  <types>
  <schema xmlns="http://www.w3.org/2001/XMLSchema" targetNamespace="http://test-uri/">
    <xsd:import namespace="http://schemas.xmlsoap.org/soap/encoding/" />
    <xsd:import namespace="http://schemas.xmlsoap.org/wsdl/" />
    $schema
  </schema>
  </types>
  <message name="testMessage">
  	<part name="testParam" type="tns:$type"/>
  </message>
	<portType name="testPortType">
		<operation name="test">
			<input message="testMessage"/>
		</operation>
	</portType>
	<binding name="testBinding" type="testPortType">
		<soap:binding style="rpc" transport="http://schemas.xmlsoap.org/soap/http"/>
		<operation name="test">
			<soap:operation soapAction="#test" style="rpc"/>
			<input>
				<soap:body use="encoding" namespace="http://test-uri/" encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
			</input>
		</operation>
	</binding>
	<service name="testService">
   <port name="testPort" binding="tns:testBinding">
     <soap:address location="test://" />
   </port>
 </service>
</definitions>
EOF;

  $filename = sys_get_temp_dir().'/'.'wsdl';
  $f = fopen($filename, 'w');
  fwrite($f, $wsdl);
  fclose($f);
  ini_set('soap.wsdl_cache_enabled', 0);
  $client = new LocalSoapClient($filename);
  unlink($filename);

  $client->__soapcall('test', vec[$param]);
  var_dump(LocalSoapClient::$input);
}
