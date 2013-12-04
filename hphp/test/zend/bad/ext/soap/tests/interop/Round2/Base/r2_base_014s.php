<?php
$param = new SoapParam(new SoapVar(array(
		new SoapVar('arg', XSD_STRING, null, null, 'varString'),
		new SoapVar('34',  XSD_INT, null, null, 'varInt'),
		new SoapVar('325.325',  XSD_FLOAT, null, null, 'varFloat')
  ),SOAP_ENC_OBJECT,"SOAPStruct","http://soapinterop.org/xsd"), "inputStruct");
$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echoStruct", array($param), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_base.inc");
echo "ok\n";
?>