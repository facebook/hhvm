<?hh
function Test($param) :mixed{

	ZendGoodExtSoapTestsBugsBug38004::$g = $param->strA."\n".$param->strB."\n";
	return ZendGoodExtSoapTestsBugsBug38004::$g;
}

class TestSoapClient extends SoapClient {
  function __construct($wsdl) {
    parent::__construct($wsdl);
    $this->server = new SoapServer($wsdl);
    $this->server->addFunction('Test');
  }

  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }
}

abstract final class ZendGoodExtSoapTestsBugsBug38004 {
  public static $g;
}
<<__EntryPoint>>
function main_entry(): void {

  $client = new TestSoapClient(dirname(__FILE__).'/bug38004.wsdl');
  $strA = 'test &amp; test';
  $strB = 'test & test';
  $res = $client->__soapcall(
    'Test',
    vec[dict['strA'=>$strA, 'strB'=>$strB]],
  );
  print_r($res);
  print_r(ZendGoodExtSoapTestsBugsBug38004::$g);
}
