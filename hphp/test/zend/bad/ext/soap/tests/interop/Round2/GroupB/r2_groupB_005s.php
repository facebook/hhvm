<?php
$param = new SoapParam(new SoapVar(array(
    new SoapVar("arg", XSD_STRING, null, null, "varString"),
    new SoapVar(34, XSD_INT, null, null, "varInt"),
    new SoapVar(325.325, XSD_FLOAT, null, null, "varFloat"),
    new SoapVar(array(
	    new SoapVar("red", XSD_STRING),
	    new SoapVar("blue", XSD_STRING),
	    new SoapVar("green", XSD_STRING),
    ), SOAP_ENC_ARRAY, "ArrayOfString", "http://soapinterop.org/xsd", 'varArray')
  ), SOAP_ENC_OBJECT, "SOAPArrayStruct", "http://soapinterop.org/xsd"), "inputStruct");
$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echoNestedArray", array($param), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>