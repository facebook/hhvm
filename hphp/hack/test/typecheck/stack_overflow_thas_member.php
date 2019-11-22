<?hh

class B {
  public float $b;
  public function __construct() {
    throw new \Exception();
  }
}

class A {
  public B $a;
  public function __construct() {
    throw new \Exception();
  }
}

function foo(): Map<string, A> {
  $m = Map {};
  $x = $m->get("a");
  if ($x === null) {
    $x = new A();
  }
  $x->a->b = 0.0;
  $m["a"] = $x;
  return $m;
}
