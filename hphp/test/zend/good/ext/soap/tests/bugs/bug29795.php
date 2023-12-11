<?hh
class LocalSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
  }

  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
    return <<<EOF
<?xml version="1.0" encoding="UTF-8"?><SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xmlns:xsd="http://www.w3.org/2001/XMLSchema"><SOAP-ENV:Body><Price><Amount>3995</Amount><CurrencyCode>USD</CurrencyCode></Price></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;
  }

}
<<__EntryPoint>>
function main_entry(): void {

  $client = new LocalSoapClient(dirname(__FILE__)."/bug29795.wsdl",dict["trace"=>1]);
  $ar=$client->__soapcall('GetPrice', vec[]);
  echo "o";
  $client = new LocalSoapClient(dirname(__FILE__)."/bug29795.wsdl",dict["trace"=>1]);
  $ar=$client->__soapcall('GetPrice', vec[]);
  echo "k\n";
}
