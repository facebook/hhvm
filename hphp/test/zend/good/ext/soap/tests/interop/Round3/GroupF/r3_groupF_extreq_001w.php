<?hh <<__EntryPoint>> function main(): void {
$client = new SoapClient(dirname(__FILE__)."/round3_groupF_extreq.wsdl",dict["trace"=>1,"exceptions"=>0]);
$client->echoString("Hello World");
echo $client->__getlastrequest();
//$HTTP_RAW_POST_DATA = $client->__getlastrequest();
//include("round3_groupF_extreq.inc");
echo "ok\n";
}
