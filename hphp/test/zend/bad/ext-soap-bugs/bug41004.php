<?php
ini_set('soap.wsdl_cache_enabled', false);

class EchoBean{
	public $mandatoryElement;
	public $optionalElement;
	
}

class EchoRequest{
	public $in;
}

class EchoResponse{
	public $out;
}

$wsdl = dirname(__FILE__)."/bug41004.wsdl";
$classmap = array('EchoBean'=>'EchoBean','echo'=>'EchoRequest','echoResponse'=>'EchoResponse');
$client = new SoapClient($wsdl, array('location'=>'test://',"classmap" => $classmap, 'exceptions'=>0, 'trace'=>1));
$echo=new EchoRequest();
$in=new EchoBean();
$in->mandatoryElement="REV";
$in->optionalElement=NULL;
$echo->in=$in;
$client->echo($echo);
echo $client->__getLastRequest();
?>