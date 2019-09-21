<?hh

<<__EntryPoint>>
function main() {
$get = $GLOBALS['_GET'];
parse_str("wsdl", inout $get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);

$x = new SoapClient(dirname(__FILE__)."/bug27722.wsdl");
echo "ok\n";
}
