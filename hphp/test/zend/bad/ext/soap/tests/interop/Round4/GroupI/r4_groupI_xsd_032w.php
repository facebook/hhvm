<?php
$hdr = new SoapHeader("http://soapinterop.org/","echoMeComplexTypeRequest", array("varInt"=>34,"varString"=>"arg","varFloat"=>12.345), 1);
$client = new SoapClient(dirname(__FILE__)."/round4_groupI_xsd.wsdl",array("trace"=>1,"exceptions"=>0));
$client->__soapCall("echoVoidSoapHeader",array(),null,$hdr);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupI_xsd.inc");
echo "ok\n";
?>