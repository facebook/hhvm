<?php
class XmlTest {

    function test_ref(&$test)
    {
    	$test = "ok";
    }

    function test($test)
    {
    }

    function run()
    {
        $ar = array();
        $this->test_ref($ar[]);
        var_dump($ar);
        $this->test($ar[]);
    }
}

$o = new XmlTest();
$o->run();
?>
