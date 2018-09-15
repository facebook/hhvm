<?hh


<<__EntryPoint>>
function main_isset_unset_superglobals() {
var_dump(isset($_GET));
var_dump(isset($GLOBALS));

$_GET = 1;
$_GET;

unset($_GET);
unset($GLOBALS);
}
