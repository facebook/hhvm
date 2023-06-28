<?hh

class Y {
  static function baz($a) :mixed{
 var_dump(static::class);
 }
}
class X {
  function foo() :mixed{
    Y::baz(static::bar());
  }
  static function bar() :mixed{
    var_dump(static::class);
  }
}

<<__EntryPoint>>
function main_1888() :mixed{
$x = new X;
$x->foo();
}
