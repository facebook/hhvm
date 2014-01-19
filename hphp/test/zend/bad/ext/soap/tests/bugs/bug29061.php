<?php
$client = new SoapClient(dirname(__FILE__)."/bug29061.wsdl", array("exceptions"=>0)); 
$client->getQuote("ibm"); 
echo "ok\n";
?>