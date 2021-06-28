<?hh

class X {
  <<__DynamicallyCallable>> public function foo($y) {
    call_user_func(varray[$y, 'foo']);
    $y::foo();
  }
}
class Y {
  <<__DynamicallyCallable>> public static function foo() {
    var_dump(__METHOD__);
    static::bar();
  }
  public static function bar() {
    var_dump(__METHOD__);
  }
}

<<__EntryPoint>>
function main_1883() {
$x = new X;
$x->foo('Y');
$x->foo(new Y);
}
