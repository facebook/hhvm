<?hh


<<__EntryPoint>>
function main_isset_unset_superglobals() {
var_dump(isset($_GET));

$_GET = 1;
$_GET;

unset($_GET);
}
