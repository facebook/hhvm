<?php
$options = array(
  "location" => "test://",
  "uri"      => "http://bricolage.sourceforge.net/Bric/SOAP/Auth",
  "trace"    => 1);

$client = new SoapClient(null, $options);

$newNS = "http://bricolage.sourceforge.net/Bric/SOAP/Story";

try {
  $client->__soapCall("list_ids", array(), array("uri" => $newNS));
} catch (Exception $e) {
  print $client->__getLastRequest();
}
?>