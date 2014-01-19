<?php
class Person {
    function Person($a=NULL, $i=NULL, $n=NULL, $m=NULL) {
        $this->Age = $a;
        $this->ID = $i;
        $this->Name = $n;
        $this->Male = $m;
    }
}
$person = new Person(32,12345,'Shane',TRUE);
$client = new SoapClient(dirname(__FILE__)."/round3_groupD_compound1.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoPerson($person);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round3_groupD_compound1.inc");
echo "ok\n";
?>