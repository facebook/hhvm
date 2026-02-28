<?hh

class X {
  private $foo;

  function __construct($a) {
    $this->foo = $a;
  }

  function foo() :mixed{ return $this->foo; }
}

function foo($q, $a) :mixed{
  $x = getX($q, $a);
  fiz($x->foo());
}

function fiz($a) :mixed{
  var_dump($a);
}
function getX($q, $a) :mixed{
  if ($q) $x = new X($a);
  return $x;
}

<<__EntryPoint>> function main(): void {
  foo(true, 1);
  foo(false, "a");
}
