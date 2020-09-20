<?hh

function Test($param) {

  DvArrayHackArrExtSoapBug38055::$g1 = $param->boolA;
  DvArrayHackArrExtSoapBug38055::$g2 = $param->boolB;
  return 1;
}

class TestSoapClient extends SoapClient {
  function __construct($wsdl) {
    parent::__construct($wsdl);
    $this->server = new SoapServer($wsdl);
    $this->server->addFunction('Test');
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }
}

abstract final class DvArrayHackArrExtSoapBug38055 {
  public static $g1;
  public static $g2;
}
<<__EntryPoint>>
function main_entry(): void {

  $client = new TestSoapClient(dirname(__FILE__).'/bug38055.wsdl');
  var_dump($client->__getfunctions());
  var_dump($client->__gettypes());
  $boolA = 1;
  $boolB = '1';
  $res = $client->__soapcall(
    'Test',
    varray[darray['boolA'=>$boolA, 'boolB'=>$boolB]],
  );
  var_dump(DvArrayHackArrExtSoapBug38055::$g1);
  var_dump(DvArrayHackArrExtSoapBug38055::$g2);
}
