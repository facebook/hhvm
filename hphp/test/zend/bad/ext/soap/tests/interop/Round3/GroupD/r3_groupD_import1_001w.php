<?php
$client = new SoapClient(dirname(__FILE__)."/round3_groupD_import1.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoString("Hello World");
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round3_groupD_import1.inc");
echo "ok\n";
?>