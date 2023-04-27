<?hh

trait T {
  abstract static function foo();
  static function bar() {
    return static::foo<>;
  }
}
class C {
  use T;
  static function foo() {
    var_dump(__METHOD__);
  }
}

<<__EntryPoint>>
function main() {
  C::bar()();
  $f = T::bar();
  echo "FAIL\n";
  $f();
}
