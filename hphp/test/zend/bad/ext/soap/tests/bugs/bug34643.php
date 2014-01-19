<?php
ini_set("soap.wsdl_cache_enabled", 0);

class fp {
	public function get_it($opt="zzz") {
		return $opt;
	}
}

class LocalSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
    $this->server->setClass('fp');
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }

}

$cl = new LocalSoapClient(dirname(__FILE__).'/bug34643.wsdl', array("trace"=>1));
print_r($cl->__getFunctions());
echo $cl->get_it("aaa")."\n";
echo $cl->get_it()."\n";
var_dump($cl->get_it(null));
?>