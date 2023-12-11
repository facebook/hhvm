<?hh

function getContinentList() :mixed{
	return dict["getContinentListResult"=>dict[
	  "schema"=>"<xsd:schema><element name=\"test\" type=\"xsd:string\"/></xsd:schema>",
	  "any"=>"<test>Hello World!</test><test>Bye World!</test>"]];
}

class LocalSoapClient extends SoapClient {
  function __construct($wsdl, $options=dict[]) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
		$this->server->addFunction("getContinentList");
  }

  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
  	echo $request;
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
  	echo $response;
    return $response;
  }
}
<<__EntryPoint>>
function main_entry(): void {
  ini_set("soap.wsdl_cache_enabled", 0);

  $client = new LocalSoapClient(dirname(__FILE__)."/bug30106.wsdl");
  var_dump($client->__getFunctions());
  var_dump($client->__getTypes());
  $x = $client->__soapcall(
    'getContinentList',
    vec[dict["AFFILIATE_ID"=>1,"PASSWORD"=>"x"]],
  );
  var_dump($x);
}
