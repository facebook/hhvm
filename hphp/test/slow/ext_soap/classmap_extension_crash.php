<?php
ini_set("soap.wsdl_cache_enabled", 0);

class A {
  public $x;
}

class B extends A {
  public $y;
}

function f($a) {
  return $a;
}

class LocalSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
    $this->server->addFunction("f");
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }
}

$client = new LocalSoapClient(
    dirname(__FILE__) . "/classmap_extension_crash.wsdl",
    array('classmap' => array('A' => 'A', 'B' => 'B'))
);
$b = new B();
$b->x = 1;
$b->y = 2;
print_r($client->f($b));
