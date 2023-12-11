<?hh

function echoLong($num) :mixed{
  return $num;
}

class LocalSoapClient extends SoapClient {

  function __construct($wsdl) {
    parent::__construct($wsdl);
    $this->server = new SoapServer($wsdl);
    $this->server->addFunction('echoLong');
  }

  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }

}

function test($num) :mixed{

  try {
	  printf("%s %0.0f\n", gettype($num), $num);
    $ret = ZendGoodExtSoapTestsBugsBug36999::$soap
      ->__soapcall('echoLong', vec[$num]);
	  printf("%s %0.0f\n", gettype($ret), $ret);
	} catch (SoapFault $ex) {
	  var_dump($ex);
	}
}

abstract final class ZendGoodExtSoapTestsBugsBug36999 {
  public static $soap;
}
<<__EntryPoint>>
function entrypoint_bug36999(): void {

  ZendGoodExtSoapTestsBugsBug36999::$soap = new LocalSoapClient(dirname(__FILE__)."/bug36999.wsdl");
  test(3706790240);
}
