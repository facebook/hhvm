<?php
class TestSoapClient extends SoapClient {
  function __doRequest($request, $location, $action, $version, $one_way = 0) {
  	echo "$request\n";
  	exit;
  }
}

$my_xml = "<array><item/><item/><item/></array>";
$client = new TestSoapClient(null, array('location' => 'test://', 'uri' => 'test://'));
$client->AnyFunction(new SoapVar($my_xml, XSD_ANYXML));
?>