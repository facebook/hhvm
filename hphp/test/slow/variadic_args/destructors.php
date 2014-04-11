<?hh

error_reporting(-1);
require_once __DIR__.'/variadic_funcs.inc';

class HasD {
  private static $counter = 0;
  private $n;
  public function __construct() {
    $this->n = self::$counter++;
  }

  public function __destruct() {
    echo __METHOD__, '(', $this->n, ")\n";
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
  echo "\n\n";
}

function test_with_exceptions() {
  echo '= ', __FUNCTION__, ' =', "\n";
  $i1 = new HasD(); $i2 = new HasD(); $i3 = new HasD();
  try {
    variadic_only_no_vv_exc($i1);
  } catch (Exception $e) { help_catch($e); }
  try {
    variadic_only_no_vv_exc($i2, $i3);
  } catch (Exception $e) { help_catch($e); }
  echo "\n\n";
}

function main() {
  test_refcount();
  test_with_exceptions();
  echo 'Done', "\n";
}
main();
