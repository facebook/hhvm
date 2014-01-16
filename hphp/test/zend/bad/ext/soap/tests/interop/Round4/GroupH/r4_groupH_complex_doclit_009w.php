<?php
class SOAPStruct {
    function SOAPStruct($s, $i, $f) {
        $this->varString = $s;
        $this->varInt = $i;
        $this->varFloat = $f;
    }
}
class BaseStruct {
    function BaseStruct($f, $s) {
        $this->structMessage = $f;
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
class MoreExtendedStruct extends ExtendedStruct {
    function MoreExtendedStruct($f, $s, $x1, $x2, $x3, $b) {
        $this->ExtendedStruct($f, $s, $x1, $x2, $x3);
        $this->booleanMessage = $b;
    }
}
$s1 = new BaseStruct(new SOAPStruct("s1",1,1.1),1);
$s2 = new ExtendedStruct(new SOAPStruct("s2",2,2.2),2,"arg",-3,5);
$s3 = new MoreExtendedStruct(new SOAPStruct("s3",3,3.3),3,"arg",-3,5,true);
$client = new SoapClient(dirname(__FILE__)."/round4_groupH_complex_doclit.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoMultipleFaults2(array("whichFault" => 3,
                                   "param1"     => $s1,
                                   "param2"     => $s2,
                                   "param3"     => $s3));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupH_complex_doclit.inc");
echo "ok\n";
?>