<?hh <<__EntryPoint>> function main(): void {
$client = new SoapClient(dirname(__FILE__)."/bug29236.wsdl");
var_dump($client->__getFunctions());
}
