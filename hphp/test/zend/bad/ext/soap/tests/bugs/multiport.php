<?php
$client = new SoapClient(dirname(__FILE__).'/multiport.wsdl', 
	array('trace' => true, 'exceptions' => false));
$response = $client->GetSessionId(array('userId'=>'user', 'password'=>'password'));
echo $client->__getLastRequest();
?>