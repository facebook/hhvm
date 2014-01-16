<?php
function Test($param) {
	return new SoapFault('Test', 'This is our fault: Ä');
}

class TestSoapClient extends SoapClient {
  function __construct($wsdl, $opt) {
    parent::__construct($wsdl, $opt);
    $this->server = new SoapServer($wsdl, $opt);
    $this->server->addFunction('Test');
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }
}

$client = new TestSoapClient(NULL, array(
    'encoding' => 'ISO-8859-1',
	'uri' => "test://",
	'location' => "test://",
	'soap_version'=>SOAP_1_2,
	'trace'=>1, 
	'exceptions'=>0));
$res = $client->Test();
echo($res->faultstring."\n");
echo($client->__getLastResponse());
?>