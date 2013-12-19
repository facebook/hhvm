<?php

$d = null;

function test($x) {
	global $d;
	$d = $x;
}

class LocalSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
    $this->server->addFunction('test');
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }

}

$x = new LocalSoapClient(dirname(__FILE__)."/bug32776.wsdl",array("trace"=>true,"exceptions"=>false)); 
var_dump($x->test("Hello"));
var_dump($d);
var_dump($x->__getLastRequest());
var_dump($x->__getLastResponse());
echo "ok\n";
?>