<?php
function my_errorhandler($errno,$errormsg) {

  ZendGoodZendTestsUnexpectedRefBug::$my_var = 0;
  return true;
}
set_error_handler("my_errorhandler");
ZendGoodZendTestsUnexpectedRefBug::$my_var = str_repeat("A",64);
try { $data = call_user_func_array("explode",array(new StdClass(), &$my_var)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
ZendGoodZendTestsUnexpectedRefBug::$my_var=array(1,2,3);
$data = call_user_func_array("implode",array(&ZendGoodZendTestsUnexpectedRefBug::$my_var, new StdClass()));
echo "Done.\n";

abstract final class ZendGoodZendTestsUnexpectedRefBug {
  public static $my_var;
}
