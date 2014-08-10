<?php
class SOAPComplexType {
    function SOAPComplexType($s, $i, $f) {
        $this->varString = $s;
        $this->varInt = $i;
        $this->varFloat = $f;
    }
}
$struct = new SOAPComplexType('arg',34,325.325);
$client = new SoapClient(dirname(__FILE__)."/round4_groupI_xsd.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoAnyType(array('inputAnyType'=>new SoapVar($struct,SOAP_ENC_OBJECT,"SOAPComplexType","http://soapinterop.org/xsd")));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupI_xsd.inc");
echo "ok\n";
?>
