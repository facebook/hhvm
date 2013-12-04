<?php
$soap = new SoapClient(dirname(__FILE__)."/bug42359.wsdl");
print_r($soap->__getTypes());
?>