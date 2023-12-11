<?hh

class MySoapClient extends SoapClient {
  public function __dorequest(
    $request,
    $location,
    $action,
    $version,
    $one_way = null
  ) :mixed{
    $dom = new DOMDocument('1.0');

    // loads the SOAP request to the Document
    $dom->loadXML($request);

    $elements = $dom->getElementsByTagName('parameters');
    foreach ($elements as $element) {
      var_dump($element->nodeValue);
      var_dump($element->getAttribute('uid'));
    }

    return '';
  }
}

class BodyType {}


<<__EntryPoint>>
function main_data_type_check() :mixed{
$client = new MySoapClient(
  __DIR__ . '/data-type-check.wdsl',
  dict[
    'classmap' => dict[
      'BodyType' => 'BodyType',
    ],
    'cache_wsdl' => WSDL_CACHE_NONE,
    'soap_version' => SOAP_1_1,
  ]
);

$recipientType = new BodyType();
$recipientType->_ = '1234567890';
$recipientType->uid = 4119859;
$client->__soapcall('test', vec[$recipientType]);
}
