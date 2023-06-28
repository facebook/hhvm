<?hh

trait T {
  function foo(int $x) :mixed{ $this->p = $x; }
  }

class X {
  use T;
  private $p = "hello";
  function bar() :mixed{
    return $this->p;
  }
}

function test($x) :mixed{
  $x->foo(42);
  var_dump($x->bar());
}


<<__EntryPoint>>
function main_private_props_022() :mixed{
test(new X);
}
