<?php
class SOAPComplexTypeComplexType {
    function SOAPComplexTypeComplexType($s, $i, $f, $c) {
        $this->varString = $s;
        $this->varInt = $i;
        $this->varFloat = $f;
        $this->varComplexType = $c;
    }
}
$struct = new SOAPComplexTypeComplexType("arg",34,12.345,NULL);
unset($struct->varComplexType);
$client = new SoapClient(dirname(__FILE__)."/round4_groupI_xsd.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoNestedComplexType(array("inputComplexType"=>$struct));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupI_xsd.inc");
echo "ok\n";
?>