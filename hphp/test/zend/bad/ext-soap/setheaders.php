<?php
$client = new SoapClient(NULL, array("location"=>"test://","uri"=>"test://",
  "exceptions"=>0,"trace"=>1));
$client->test();
echo $client->__getLastRequest();
$client->__setSoapHeaders(new SoapHeader("test://","HDR1"));
$client->test();
echo $client->__getLastRequest();
$client->test();
echo $client->__getLastRequest();
$client->__setSoapHeaders();
$client->test();
echo $client->__getLastRequest();
$client->__setSoapHeaders(array(new SoapHeader("test://","HDR1"),new SoapHeader("test://","HDR2")));
$client->test();
echo $client->__getLastRequest();
$h = array(new SoapHeader("test://","HDR0"));
$client->__soapCall("test", array(), null, $h);
echo $client->__getLastRequest();
?>