<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("wsdl", inout $get);
\HH\global_set('_GET', $get);

$x = new SoapClient(dirname(__FILE__)."/bug27722.wsdl");
echo "ok\n";
}
