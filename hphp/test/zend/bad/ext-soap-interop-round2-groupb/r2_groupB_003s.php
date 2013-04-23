<?php
$param = new SoapParam(new SoapVar(array(
    new SoapVar(array(
      new SoapVar('row0col0', XSD_STRING),
      new SoapVar('row0col1', XSD_STRING),
      new SoapVar('row0col2', XSD_STRING)
    ), SOAP_ENC_ARRAY),
    new SoapVar(array(
      new SoapVar('row1col0', XSD_STRING),
      new SoapVar('row1col1', XSD_STRING),
      new SoapVar('row1col2', XSD_STRING)
    ), SOAP_ENC_ARRAY)
  ), SOAP_ENC_ARRAY, "ArrayOfString2D", "http://soapinterop.org/xsd"),"input2DStringArray");
$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echo2DStringArray", array($param), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>