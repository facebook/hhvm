<?hh

if (isset($g)) {
  include '1488-1.inc';
}
else {
  include '1488-2.inc';
}
class Z extends Y {
  static function foo() {
    var_dump(__METHOD__);
  }
  static function bar() {
    X::foo();
  }
}
Z::bar();
