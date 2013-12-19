<?php
$param = (object)array(
  'varString' => "arg",
  'varInt' => 34,
  'varFloat' => 123.45,
  'varStruct' => (object)array(
    'varString' => "arg2",
    'varInt' => 342,
    'varFloat' => 123.452,
  ));

$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echoNestedStruct", array($param), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>