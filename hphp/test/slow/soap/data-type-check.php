<?php

class MySoapClient extends SoapClient {
  public function __doRequest(
    $request,
    $location,
    $action,
    $version,
    $one_way = null
  ) {
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

$client = new MySoapClient(
  __DIR__ . '/data-type-check.wdsl',
  array(
    'classmap' => array(
      'BodyType' => 'BodyType',
    ),
    'cache_wsdl' => WSDL_CACHE_NONE,
    'soap_version' => SOAP_1_1,
  )
);

class BodyType extends stdClass {}

$recipientType = new BodyType();
$recipientType->_ = '1234567890';
$recipientType->uid = 4119859;
$client->test($recipientType);
