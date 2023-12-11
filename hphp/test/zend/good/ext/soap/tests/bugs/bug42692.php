<?hh

function checkAuth($peid,$auth) :mixed{
	return $peid;
}

class TestSoap extends SoapClient {

	function __construct($wsdl, $options) {
		parent::__construct($wsdl, $options);
		$this->server = new SoapServer($wsdl, $options);
		$this->server->addFunction("checkAuth");
	}

	function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
		ob_start();
		$this->server->handle($request);
		$response = ob_get_contents();
		ob_end_clean();
		return $response;
	}
}
<<__EntryPoint>>
function main_entry(): void {
  ini_set('soap.wsdl_cache_enabled','0');

  $client = new TestSoap(dirname(__FILE__) . "/bug42692.wsdl", dict["trace"=>1]);
  try {
  	$result = $client->__soapcall('checkAuth', vec[1,"two"]);
  	echo "Auth for 1 is $result\n";
  } catch (Exception $e) {
  	echo $e->getMessage();
  }
}
