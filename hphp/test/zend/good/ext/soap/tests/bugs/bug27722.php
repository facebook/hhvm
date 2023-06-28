<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("wsdl", inout $get);
\HH\global_set('_GET', $get);
$_REQUEST = array_merge($_REQUEST, $_GET);

$x = new SoapClient(dirname(__FILE__)."/bug27722.wsdl");
echo "ok\n";
}
