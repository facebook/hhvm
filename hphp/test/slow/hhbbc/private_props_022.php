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

test(new X);
