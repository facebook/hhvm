<?hh

abstract final class ZendBug54265 {
  public static $my_var = array();
}
function my_errorhandler($errno,$errormsg) {
  ZendBug54265::$my_var = 0;
  echo "EROOR: $errormsg\n";
}
<<__EntryPoint>>
function ZendBug54265() {
set_error_handler("my_errorhandler");
try {
  ZendBug54265::$my_var = str_repeat("A",ZendBug54265::$my_var[0]->errormsg = "xyz");
} catch (Exception $e) {
  var_dump($e->getMessage());
}
echo "ok\n";
}
