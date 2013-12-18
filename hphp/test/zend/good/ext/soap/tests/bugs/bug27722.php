<?php
parse_str("wsdl", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

$x = new SoapClient(dirname(__FILE__)."/bug27722.wsdl");
echo "ok\n";
?>