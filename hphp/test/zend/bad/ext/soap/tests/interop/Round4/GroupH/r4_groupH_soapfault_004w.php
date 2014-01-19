<?php
$hdr = new SoapHeader("http://soapinterop.org/wsdl", "UnknownHeaderRequest", "Hello World", 1);
$client = new SoapClient(dirname(__FILE__)."/round4_groupH_soapfault.wsdl",array("trace"=>1,"exceptions"=>0));
$client->__soapCall("echoVersionMismatchFault",array(), null, $hdr);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupH_soapfault.inc");
echo "ok\n";
?>