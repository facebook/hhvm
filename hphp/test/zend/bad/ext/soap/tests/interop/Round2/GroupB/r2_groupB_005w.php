<?php
$param = (object)array(
  'varString'=>'arg',
  'varInt'=>34,
  'varFloat'=>325.325,
  'varArray' => array('red','blue','green'));
$client = new SoapClient(dirname(__FILE__)."/round2_groupB.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoNestedArray($param);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>