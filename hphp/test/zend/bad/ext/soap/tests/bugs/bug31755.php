<?php
$client=new SOAPClient(null, array('location' => 'http://localhost',
'uri' => 'myNS', 'exceptions' => false, 'trace' => true));

$header = new SOAPHeader(null, 'foo', 'bar');

$response= $client->__call('function', array(), null, $header);

print $client->__getLastRequest();
?>