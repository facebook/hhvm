<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("wsdl", inout $get);
\HH\global_set('_GET', $get);
\HH\global_set('_REQUEST', array_merge(\HH\global_get('_REQUEST'), \HH\global_get('_GET')));

$x = new SoapClient(dirname(__FILE__)."/bug27722.wsdl");
echo "ok\n";
}
