<?hh

var_dump(isset($_GET));
var_dump(isset($GLOBALS));

$_GET = 1;
$_GET;

unset($_GET);
unset($GLOBALS);
