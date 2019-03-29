<?php
/**
 * This test is a modified version of the original test, bug54265.php, which
 * has been moved to 'bad'. The intent of the test is to ensure that it does
 * not crash when $my_var gets reassigned in the custom error handler.
 *
 * The results are different for HHVM and PHP 5 due to the following reason.
 * In PHP 5, the second argument of str_repeat becomes null, which is then
 * cast into an int. As a result, $my_var is 0 instead of null.
 *
 */
function my_errorhandler($errno, $errormsg) {

  ZendGoodZendTestsBug54265Modified::$my_var = 0;
  echo "EROOR: $errormsg\n";
  return null;
}
set_error_handler("my_errorhandler");
try { $my_var = array(); $my_var = str_repeat("A", $my_var[0]->errormsg = "xyz"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(ZendGoodZendTestsBug54265Modified::$my_var);

abstract final class ZendGoodZendTestsBug54265Modified {
  public static $my_var;
}
