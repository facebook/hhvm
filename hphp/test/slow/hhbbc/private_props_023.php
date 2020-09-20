<?hh

trait T {
  function foo(int $x) {
    return function() use($x) { $this->p = $x; };
  }
}

class X {
  use T;
  private $p = "hello";
  function bar() {
    return $this->p;
  }
}

function test($x) {
  $f = $x->foo(42);
  $f();
  var_dump($x->bar());
}


<<__EntryPoint>>
function main_private_props_023() {
test(new X);
}
