<?hh

class Y {
  static function baz($a) {
 var_dump(static::class);
 }
}
class X {
  function foo() {
    Y::baz(static::bar());
  }
  static function bar() {
    var_dump(static::class);
  }
}

<<__EntryPoint>>
function main_1888() {
$x = new X;
$x->foo();
}
