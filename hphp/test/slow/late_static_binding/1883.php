<?hh

class X {
  <<__DynamicallyCallable>> public function foo($y) :mixed{
    call_user_func(vec[$y, 'foo']);
    $y::foo();
  }
}
class Y {
  <<__DynamicallyCallable>> public static function foo() :mixed{
    var_dump(__METHOD__);
    static::bar();
  }
  public static function bar() :mixed{
    var_dump(__METHOD__);
  }
}

<<__EntryPoint>>
function main_1883() :mixed{
$x = new X;
$x->foo('Y');
$x->foo(new Y);
}
