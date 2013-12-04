<?php
class SOAPMultiOccursComplexType {
    function SOAPMultiOccursComplexType($s, $i, $f, $c) {
        $this->varString = $s;
        $this->varInt = $i;
        $this->varFloat = $f;
        $this->varMultiOccurs = $c;
    }
}
$struct = new SOAPMultiOccursComplexType("arg",34,12.345,array("red","green","blue"));
$client = new SoapClient(dirname(__FILE__)."/round4_groupI_xsd.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoNestedMultiOccurs(array("inputComplexType"=>$struct));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupI_xsd.inc");
echo "ok\n";
?>