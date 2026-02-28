<?hh

trait T {
  function foo(int $x) :mixed{
    return function() use($x) { $this->p = $x; };
  }
}

class X {
  use T;
  private $p = "hello";
  function bar() :mixed{
    return $this->p;
  }
}

function test($x) :mixed{
  $f = $x->foo(42);
  $f();
  var_dump($x->bar());
}


<<__EntryPoint>>
function main_private_props_023() :mixed{
test(new X);
}
