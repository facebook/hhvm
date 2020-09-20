<?hh

class Z extends Y {
  static function foo() {
    var_dump(__METHOD__);
  }
  static function bar() {
    X::foo();
  }
}
