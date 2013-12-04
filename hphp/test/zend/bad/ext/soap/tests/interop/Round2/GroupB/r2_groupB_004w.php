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

$client = new SoapClient(dirname(__FILE__)."/round2_groupB.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoNestedStruct($param);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>