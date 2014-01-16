<?php
class TestSoapClient extends SoapClient {
  function __doRequest($request, $location, $action, $version, $one_way = 0) {
  	echo $request;
  	exit;
	}
}

ini_set("soap.wsdl_cache_enabled", 0);
$client = new TestSoapClient(dirname(__FILE__).'/bug32941.wsdl', array("trace" => 1, 'exceptions' => 0));
$ahoj = $client->echoPerson(array("name"=>"Name","surname"=>"Surname"));
echo "ok\n";
?>