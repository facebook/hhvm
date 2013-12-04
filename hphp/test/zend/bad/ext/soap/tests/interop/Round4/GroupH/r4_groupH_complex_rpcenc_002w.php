<?php
class BaseStruct {
    function BaseStruct($f, $s) {
        $this->floatMessage = $f;
        $this->shortMessage = $s;
    }
}
$struct = new BaseStruct(12.345,12);
$client = new SoapClient(dirname(__FILE__)."/round4_groupH_complex_rpcenc.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoBaseStructFault($struct);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupH_complex_rpcenc.inc");
echo "ok\n";
?>