<?hh <<__EntryPoint>> function main(): void {
$client = new SoapClient(dirname(__FILE__).'/bug28985.wsdl', dict['trace'=>1]);
var_dump($client->__getTypes());
}
