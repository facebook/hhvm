<?php
$client = new SoapClient(dirname(__FILE__)."/round3_groupD_doclitparams.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoStringArray(array("param0"=>array("one","two","three")));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round3_groupD_doclitparams.inc");
echo "ok\n";
?>
