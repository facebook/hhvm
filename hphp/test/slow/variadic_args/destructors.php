<?hh

error_reporting(-1);
require_once __DIR__.'/variadic_funcs.inc';

class HasD {
  private static $counter = 0;
  private $name;
  public function __construct($name = 'HasD') {
    $this->name = $name.' '.(self::$counter++);
  }

  public function __destruct() {
    echo __METHOD__, '(', $this->name, ")\n";
  }
}

function help_catch(Exception $e) {
  echo get_class($e), ': ', $e->getMessage(), "\n";
}

function test_refcount() {
  echo '= ', __FUNCTION__, ' =', "\n";
  variadic_only_no_vv(new HasD());
  variadic_only_no_vv(new HasD(), new HasD());
  variadic_only(new HasD());
  variadic_only(new HasD(), new HasD());
  echo "\n";

  echo '* cuf *', "\n";
  call_user_func('variadic_only_no_vv', new HasD());
  call_user_func('variadic_only_no_vv', new HasD(), new HasD());

  echo '* cufa *', "\n";
  call_user_func_array('variadic_only_no_vv', [new HasD()]);
  call_user_func_array('variadic_only_no_vv', [new HasD(), new HasD()]);
  call_user_func_array('variadic_only', [new HasD()]);
  call_user_func_array('variadic_only', [new HasD(), new HasD()]);

  echo "Exiting ", __FUNCTION__, "\n";
}

function test_with_exceptions() {
  // the unset($e) here is to account for the behavior of the exception
  // hanging on to arguments when arguments are included in the backtrace.
  echo '= ', __FUNCTION__, ' =', "\n";
  $i1 = new HasD('i1'); $i2 = new HasD('i2'); $i3 = new HasD('i3');
  try {
    variadic_only_no_vv_exc($i1);
  } catch (Exception $e) { help_catch($e); unset($e); }
  try {
    variadic_only_no_vv_exc($i2, $i3);
  } catch (Exception $e) { help_catch($e); unset($e); }

  echo '* cuf *', "\n";
  try {
    call_user_func('variadic_only_no_vv_exc', new HasD());
  } catch (Exception $e) { help_catch($e); unset($e); }
  try {
    call_user_func('variadic_only_no_vv_exc', new HasD(), new HasD());
  } catch (Exception $e) { help_catch($e); unset($e); }

  echo '* cufa *', "\n";
  try {
    call_user_func_array('variadic_only_no_vv_exc', [new HasD()]);
  } catch (Exception $e) { help_catch($e); unset($e); }
  try {
    call_user_func_array('variadic_only_no_vv_exc', [new HasD(), new HasD()]);
  } catch (Exception $e) { help_catch($e); unset($e); }

  echo "Exiting ", __FUNCTION__, "\n";
}

function main() {
  test_refcount();
  test_with_exceptions();
  echo 'Done', "\n";
}
main();
