<?php

class hello_world {   
  public function hello($to) {
    return 'Hello ' . $to;
  }    
}

class LocalSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
    $this->server->setClass('hello_world');;
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }

}

$client = new LocalSoapClient(dirname(__FILE__)."/bug29844.wsdl", array("trace"=>1)); 
var_dump($client->hello('davey'));
?>