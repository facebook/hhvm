<?php
$lo_soap = new SoapClient(dirname(__FILE__)."/bug36614.wsdl");
echo "ok\n";
?>