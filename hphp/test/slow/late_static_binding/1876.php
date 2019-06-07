<?hh

class X {
  function f() {
    $y = new Y;
    $y->bar();
    static::g();
    $y->bar();
    self::g();
    Y::foo() && static::g();
  }
  static function g() {
 var_dump(__CLASS__);
 }
}
class Y extends X {
  static function g() {
    var_dump(__CLASS__);
  }
  static function foo() {
    return true;
  }
  function bar() {
    return false;
  }
}
function test() {
  $x = new X;
  $y = new Y;
  $x->f();
  $y->f();
}

<<__EntryPoint>>
function main_1876() {
test();
}
