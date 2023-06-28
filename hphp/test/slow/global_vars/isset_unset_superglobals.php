<?hh


<<__EntryPoint>>
function main_isset_unset_superglobals() :mixed{
var_dump(isset($_GET));

$_GET = 1;
$_GET;

unset($_GET);
}
