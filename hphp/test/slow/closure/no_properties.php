<?hh


<<__EntryPoint>>
function main_no_properties() {
set_error_handler(function($errno, $errmsg) {
  echo "$errmsg\n";
}, E_RECOVERABLE_ERROR);

$func = function() {};

isset($func->a);
unset($func->a);
$func->a;
$func->a = 10;
}
