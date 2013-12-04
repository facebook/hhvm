<?php
class BaseStruct {
    function BaseStruct($f, $s) {
        $this->floatMessage = $f;
        $this->shortMessage = $s;
    }
}
class ExtendedStruct extends BaseStruct {
    function ExtendedStruct($f, $s, $x1, $x2, $x3) {
        $this->BaseStruct($f,$s);
        $this->stringMessage = $x1;
        $this->intMessage = $x2;
        $this->anotherIntMessage = $x3;
    }
}
$struct = new ExtendedStruct(12.345,12,"arg",-3,5);
$client = new SoapClient(dirname(__FILE__)."/round4_groupH_complex_rpcenc.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoExtendedStructFault($struct);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupH_complex_rpcenc.inc");
echo "ok\n";
?>