<?hh

<<__EntryPoint>>
function main() {
$get = $GLOBALS['_GET'];
parse_str("a=1&b=&c=3", inout $get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);
echo $_GET['a'];
echo $_GET['b'];
echo $_GET['c'];
}
