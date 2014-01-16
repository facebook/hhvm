<?php

function EchoString($s) {
  return $s;
}

class LocalSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
    $this->server->addFunction('EchoString');
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }

}

$client = new LocalSoapClient(dirname(__FILE__)."/bug34453.wsdl", array("trace"=>1)); 
$client->EchoString(array("value"=>"hello","lang"=>"en"));
echo $client->__getLastRequest();
echo $client->__getLastResponse();
echo "ok\n";
?>