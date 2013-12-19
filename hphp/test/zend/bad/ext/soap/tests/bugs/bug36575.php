<?php
abstract class CT_A1 {
	public $var1;
}

class CT_A2 extends CT_A1 {
	public $var2;
}

class CT_A3 extends CT_A2 {
	public $var3;
}

// returns A2 in WSDL
function test( $a1 ) {
	$a3 = new CT_A3();
	$a3->var1 = $a1->var1;
	$a3->var2 = "var two";
	$a3->var3 = "var three";
	return $a3;
}

$classMap = array("A1" => "CT_A1", "A2" => "CT_A2", "A3" => "CT_A3");

$client = new SoapClient(dirname(__FILE__)."/bug36575.wsdl", array("trace" => 1, "exceptions" => 0, "classmap" => $classMap));
$a2 = new CT_A2();
$a2->var1 = "one";
$a2->var2 = "two";
$client->test($a2);

$soapRequest = $client->__getLastRequest();

echo $soapRequest;

$server = new SoapServer(dirname(__FILE__)."/bug36575.wsdl", array("classmap" => $classMap));
$server->addFunction("test");
$server->handle($soapRequest);
echo "ok\n";
?>