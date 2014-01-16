<?php
$client = new SoapClient(dirname(__FILE__)."/round4_groupH_soapfault.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoVersionMismatchFault();
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupH_soapfault.inc");
echo "ok\n";
?>