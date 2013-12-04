<?php
class Person {
    function Person($a=NULL, $i=NULL, $n=NULL, $m=NULL) {
        $this->Age = $a;
        $this->ID = $i;
        $this->Name = $n;
        $this->Male = $m;
    }
}
class Employee {
    function Employee($person=NULL,$id=NULL,$salary=NULL) {
        $this->person = $person;
        $this->ID = $id;
        $this->salary = $salary;
    }
}
$person = new Person(32,12345,'Shane',TRUE);
$employee = new Employee($person,12345,1000000.00);

$client = new SoapClient(dirname(__FILE__)."/round3_groupD_compound2.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoEmployee($employee);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round3_groupD_compound2.inc");
echo "ok\n";
?>