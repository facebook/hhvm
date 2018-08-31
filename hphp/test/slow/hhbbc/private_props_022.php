<?hh

trait T {
  function foo(int $x) { $this->p = $x; }
  }

class X {
  use T;
  private $p = "hello";
  function bar() {
    return $this->p;
  }
}

function test($x) {
  $x->foo(42);
  var_dump($x->bar());
}


<<__EntryPoint>>
function main_private_props_022() {
test(new X);
}
