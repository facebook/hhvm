<?php
$hdr = new SoapHeader("http://soapinterop.org/xsd","Header1", array("int"=>34,"string"=>"arg"));
$client = new SoapClient(dirname(__FILE__)."/round3_groupF_headers.wsdl",array("trace"=>1,"exceptions"=>0));
$client->__soapCall("echoString",array("Hello World"),null,$hdr);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round3_groupF_headers.inc");
echo "ok\n";
?>