<?hh

class X {
  static function foo() :mixed{
    var_dump(__METHOD__, static::class);
  }
  function bar() :mixed{
    var_dump(__METHOD__, static::class);
  }
}
class Y extends X {
}
class Z extends X {
  static function foo() :mixed{
    var_dump(__METHOD__, static::class);
  }
}
function test($x, $o) :mixed{
  $x->getMethod('foo')->invoke($o);
  $x->getMethod('foo')->invoke(null);
  $x->getMethod('bar')->invoke($o);
}

<<__EntryPoint>>
function main_1887() :mixed{
test(new ReflectionClass('X'), new X);
test(new ReflectionClass('Y'), new X);
test(new ReflectionClass('X'), new X);
test(new ReflectionClass('Z'), new Z);
call_user_func(vec[new Y, 'X::foo']);
call_user_func(vec[new Z, 'X::foo']);
}
