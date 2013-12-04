<?php
$client = new SOAPClient(dirname(__FILE__).'/bug28985.wsdl', array('trace'=>1));
var_dump($client->__getTypes());
?>