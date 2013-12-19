<?php
function test(){
  return 123.456;
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
                                   "trace"=>1)); 
setlocale(LC_ALL,"sv_SE","sv_SE.ISO8859-1");
var_dump($x->test());
echo $x->__getLastResponse();
setlocale(LC_ALL,"en_US","en_US.ISO8859-1");
var_dump($x->test());
echo $x->__getLastResponse();