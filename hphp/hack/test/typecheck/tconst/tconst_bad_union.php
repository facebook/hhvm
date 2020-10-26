<?hh // strict
// keep

abstract class A {}

class A1 extends A {
  public function makeme(): void {}
}
class A2 extends A {
  public function fatal(): void {}
}

abstract class C {
  abstract const type T as A;
  abstract public function __construct(this::T $x);
}

final class C1 extends C {
  const type T = A1;
  public function __construct(this::T $x) { $x->makeme(); }
}
final class C2 extends C {
  const type T = A2;
  public function __construct(this::T $x) { $x->fatal(); }
}

function test(A $x): C {
  if ($x is A2) {
    $c = C1::class;
  } else if ($x is A1) {
    $c = C2::class;
  } else {
    while (true) {}
  }
  return new $c($x);
}

// oops
// test(new A1());
