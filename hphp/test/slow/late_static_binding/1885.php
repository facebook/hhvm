<?hh

class A {
  <<__DynamicallyCallable>> public static function foo() :mixed{
    var_dump(static::class);
  }
  public static function bar() :mixed{
    $x = new B;
    $x->fiz(self::foo());
  }
  function fiz($x) :mixed{
}
}
class B extends A {
}

<<__EntryPoint>>
function main_1885() :mixed{
$array = varray['foo'];
array_map('B::foo', $array);
call_user_func('B::foo');
call_user_func(varray['B', 'foo']);
A::bar();
}
