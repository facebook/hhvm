<?hh
function my_errorhandler($errno,$errormsg) {
  global $my_var;
  $my_var = 0;
  echo "EROOR: $errormsg\n";
}
set_error_handler("my_errorhandler");
try {
  $my_var = str_repeat("A",$my_var[0]->errormsg = "xyz");
} catch (Exception $e) {
  var_dump($e->getMessage());
}
echo "ok\n";
