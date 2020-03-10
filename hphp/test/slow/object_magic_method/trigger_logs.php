<?hh

class A {
  public function __set($in, $val): bool {
    return true;
  }
  public function __get($in): mixed {
    return null;
  }
  public function __isset($in): bool {
    return true;
  }
  public function __unset($in): void {
    return;
  }
  public function __call($f, $args): mixed {
    if (ini_get('hhvm.no_use_magic_methods')) {
      trigger_error(
        "Invoking A::$f via magic __call",
        E_USER_WARNING
      );
    }
    return null;
  }
 }

 <<__EntryPoint>>
function main_748() {
  $obj = new A();
  $obj->test();
  isset($obj->test);
  unset($obj->test);
  $obj->test;
  $obj->test = 'test';
}
