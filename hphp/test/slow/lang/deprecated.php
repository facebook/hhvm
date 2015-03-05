<?hh

function err_handler($errno, $errstr, $file, $line) {
  if ($errno === E_DEPRECATED) {
    echo 'E_DEPRECATED', ': ', $errstr, ' for ', $file, ':', $line, "\n";
    return true;
  } else if ($errno === E_USER_DEPRECATED) {
    echo 'E_USER_DEPRECATED', ': ', $errstr, ' for ', $file, ':', $line, "\n";
    return true;
  }
  throw new Exception($errstr);
}
error_reporting(-1);
set_error_handler('err_handler');

trait Tr {
  <<__Deprecated('message')>>
  public function meth() { echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public static function stMeth() { echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public static function __callStatic($name, $args) { echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public function __call($name, $args) { echo __METHOD__, "\n"; }
}

class C {
  <<__Deprecated('message')>>
  public function meth() { echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public static function stMeth() { echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public static function __callStatic($name, $args) { echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public function __call($name, $args) { echo __METHOD__, "\n"; }
}

class C_T {
  use Tr;
}

<<__Deprecated('message')>>
function f() { echo __METHOD__, "\n"; }
<<__Deprecated('message')>>
function f_gen() { yield null; echo __METHOD__, "\n"; }
<<__Memoize, __Deprecated('message')>>
function fMem() {
  echo __METHOD__, "\n";
}

function basic() {
  echo '= ', __FUNCTION__, " =", "\n";
  f();
  C::stMeth();
  C::viaCallStatic();
  $inst = new C();
  $inst->meth();
  $inst->viaCall();
  f_gen();
  echo "\n";
}

function memoized() {
  echo '= ', __FUNCTION__, " =", "\n";
  fMem();
  fMem();
  echo "\n";
}

function builtin() {
  echo '= ', __FUNCTION__, " =", "\n";
  ereg('foobar', 'foo');
  echo "\n";
}

function via_trait() {
  echo '= ', __FUNCTION__, " =", "\n";
  // f();
  C_T::stMeth();
  C_T::viaCallStatic();
  $inst = new C_T();
  $inst->meth();
  $inst->viaCall();
  echo "\n";
}

function main() {
  basic();
  via_trait();
  memoized();
  builtin();
  echo 'Done', "\n";
}

main();
