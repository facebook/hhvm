<?php
function test() {
  return "жше";
//  return utf8_encode("жше");
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

$x = new LocalSoapClient(NULL,array('location'=>'test://', 
                                    'uri'=>'http://testuri.org',
                                    'encoding'=>'ISO-8859-1')); 
var_dump($x->test());
echo "ok\n";
?>