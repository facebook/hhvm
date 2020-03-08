<?hh // strict

namespace NS_intrinsics_invariant;

interface I {
  public function foo(): void;
}

class A implements I {
  public function foo(): void {
    echo "A";
  }
}

class B implements I {
  public function foo(): void {
    echo "B";
  }

  public function yay(): void {
    echo "B->yay!";
  }
}

function baz(int $a): I {
  return $a === 1 ? new A() : new B();
}

function bar(): B {
  $iface = baz(2);
  invariant($iface instanceof B, "Object must have type B");
  $iface->yay();
  return $iface;
}

function main(?int $p): void {
  bar();

  invariant(!is_null($p), "Value can't be null");
  $v = $p << 2;

  $max = 100;
  invariant(!is_null($p) && $p <= $max, "Value %d must be <= %d", $p, $max);
}

//main(123);
//main(null);
