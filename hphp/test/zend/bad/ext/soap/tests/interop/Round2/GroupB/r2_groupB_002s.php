<?php
$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echoSimpleTypesAsStruct", array(
  new SoapParam(new SoapVar("arg",XSD_STRING), "inputString"),
  new SoapParam(new SoapVar(34,XSD_INT), "inputInteger"),
  new SoapParam(new SoapVar(34.345,XSD_FLOAT), "inputFloat")), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>