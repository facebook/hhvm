<?hh

class X {
  public $bar = 5;

  private static $fooV;
  function foo() {
    if (!self::$fooV) self::$fooV = $this;
    return self::$fooV;
  }
}

abstract final class FooStatics {
  public static $v;
}
function foo() {
  if (!FooStatics::$v) FooStatics::$v = new X;
  return FooStatics::$v;
}
function test() {
  $x = new X;
  var_dump($x->foo()->bar);
  var_dump($x->foo()->bar);
  var_dump($x->foo()->bar);
  var_dump(foo()->bar);
  foo()->bar = 6;
  var_dump(foo()->bar);
  foo()->bar = 7;
  var_dump(foo()->bar);
  foo()->bar = 8;
  var_dump(foo()->bar);
}

<<__EntryPoint>>
function main_719() {
test();
}
