<?php
$soapClient = new SoapClient(__DIR__ . '/bug50997.wsdl', array('trace' => 1, 'exceptions'=>0));
$params = array('code'=>'foo');
$soapClient->newOperation($params);
echo $soapClient->__getLastRequest();
?>