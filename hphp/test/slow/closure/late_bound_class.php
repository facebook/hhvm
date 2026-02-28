<?hh

class X {
  static function foo() :mixed{
    var_dump(static::class);
    return function() {
      var_dump(static::class);
    };
  }
  function bar_nonstatic() :mixed{
    var_dump(static::class);
    return function() {
      var_dump(static::class);
    };
  }
}
class Y extends X {}

function test() :mixed{
  $a = X::foo();
  $a();
  $a = Y::foo();
  $a();

  $x = new X;
  $a = $x->bar_nonstatic();
  $a();
  $x = new Y;
  $a = $x->bar_nonstatic();
  $a();
}


<<__EntryPoint>>
function main_late_bound_class() :mixed{
test();
}
