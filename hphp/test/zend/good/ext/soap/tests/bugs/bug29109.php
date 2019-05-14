<?php <<__EntryPoint>> function main() {
$client = new SoapClient(dirname(__FILE__)."/bug29109.wsdl");
var_dump($client->__getFunctions());
}
