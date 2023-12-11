<?hh

class foo {
	public    $a="a";
	private   $b="b";
	protected $c="c";
}

function test($x) :mixed{
  return $x;
}

class LocalSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
    $this->server->addFunction('test');
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
  ini_set("soap.wsdl_cache_enabled", 0);

  $x = new LocalSoapClient(dirname(__FILE__)."/bug30928.wsdl",
                           dict[]);
  var_dump($x->__soapcall('test', vec[new foo()]));

  $x = new LocalSoapClient(dirname(__FILE__)."/bug30928.wsdl",
                           dict["classmap" => dict['testType'=>'foo']]);
  var_dump($x->__soapcall('test', vec[new foo()]));

  echo "ok\n";
}
