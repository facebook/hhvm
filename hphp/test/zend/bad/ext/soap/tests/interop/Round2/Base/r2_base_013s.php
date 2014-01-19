<?php
$param =  new SoapParam(new SoapVar(array(
    new SoapVar(1.3223, XSD_FLOAT),
    new SoapVar(34.2, XSD_FLOAT),
    new SoapVar(325.325, XSD_FLOAT)
  ), SOAP_ENC_ARRAY, "ArrayOffloat","http://soapinterop.org/xsd"), "inputFloatArray");
$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echoFloatArray", array($param), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_base.inc");
echo "ok\n";
?>