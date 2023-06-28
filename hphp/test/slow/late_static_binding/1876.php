<?hh

class X {
  function f() :mixed{
    $y = new Y;
    $y->bar();
    static::g();
    $y->bar();
    self::g();
    Y::foo() && static::g();
  }
  static function g() :mixed{
 var_dump(__CLASS__);
 }
}
class Y extends X {
  static function g() :mixed{
    var_dump(__CLASS__);
  }
  static function foo() :mixed{
    return true;
  }
  function bar() :mixed{
    return false;
  }
}
function test() :mixed{
  $x = new X;
  $y = new Y;
  $x->f();
  $y->f();
}

<<__EntryPoint>>
function main_1876() :mixed{
test();
}
