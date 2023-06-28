<?hh

/**
 * Class with two singleton instances.
 */
class C {
  private static $instanceSame = null;
  private static $instanceNSame = null;

  public function __construct() {
    echo "C!\n";
  }

  public static function getSame() :mixed{
    if (self::$instanceSame === null) {
      echo __FUNCTION__ . " made a ";
      self::$instanceSame = new C();
    }
    return self::$instanceSame;
  }

  public static function getNSame() :mixed{
    if (self::$instanceNSame !== null) {
      return self::$instanceNSame;
    }
    echo __FUNCTION__ . " made a ";
    self::$instanceNSame = new C();
    return self::$instanceNSame;
  }
}

abstract final class CheckNullSameStatics {
  public static $instance = null;
}

/**
 * Toplevel functions with singleton statics.
 *
 * The *_loop functions are the same as the normal versions; they're duplicated
 * only to get different statics.
 */
function check_null_same() :mixed{
  if (CheckNullSameStatics::$instance === null) {
    echo __FUNCTION__ . " made a ";
    CheckNullSameStatics::$instance = new C;
  }
  return CheckNullSameStatics::$instance;
}

abstract final class CheckNullSameLoopStatics {
  public static $instance = null;
}

function check_null_same_loop() :mixed{
  if (CheckNullSameLoopStatics::$instance === null) {
    echo __FUNCTION__ . " made a ";
    CheckNullSameLoopStatics::$instance = new C;
  }
  return CheckNullSameLoopStatics::$instance;
}

abstract final class CheckNullNsameStatics {
  public static $instance = null;
}

function check_null_nsame() :mixed{
  if (CheckNullNsameStatics::$instance !== null) {
    return CheckNullNsameStatics::$instance;
  }
  echo __FUNCTION__ . " made a ";
  CheckNullNsameStatics::$instance = new C;
  return CheckNullNsameStatics::$instance;
}

abstract final class CheckNullNsameLoopStatics {
  public static $instance = null;
}

function check_null_nsame_loop() :mixed{
  if (CheckNullNsameLoopStatics::$instance !== null) {
    return CheckNullNsameLoopStatics::$instance;
  }
  echo __FUNCTION__ . " made a ";
  CheckNullNsameLoopStatics::$instance = new C;
  return CheckNullNsameLoopStatics::$instance;
}

/**
 * Tests.
 */
function run_null_same() :mixed{
  var_dump(check_null_same());
  var_dump(check_null_same());
}

function run_null_same_loop() :mixed{
  for ($i = 0; $i < 2; ++$i) {
    var_dump(check_null_same_loop());
  }
}

function run_null_nsame() :mixed{
  var_dump(check_null_nsame());
  var_dump(check_null_nsame());
}

function run_null_nsame_loop() :mixed{
  for ($i = 0; $i < 2; ++$i) {
    var_dump(check_null_nsame_loop());
  }
}

function run_sprop_get_same() :mixed{
  var_dump(C::getSame());
  var_dump(C::getSame());
}

function run_sprop_get_nsame() :mixed{
  var_dump(C::getNSame());
  var_dump(C::getNSame());
}


<<__EntryPoint>>
function main_singleton() :mixed{
run_null_same();
run_null_same_loop();
run_null_nsame();
run_null_nsame_loop();
run_sprop_get_same();
run_sprop_get_nsame();
}
