<?php
$param =  new SoapParam(new SoapVar(array(
    new SoapVar(1,XSD_INT),
    new SoapVar(234324324,XSD_INT),
    new SoapVar(2,XSD_INT)
  ), SOAP_ENC_ARRAY, "ArrayOfint","http://soapinterop.org/xsd"), "inputIntegerArray");
$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echoIntegerArray", array($param), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_base.inc");
echo "ok\n";
?>