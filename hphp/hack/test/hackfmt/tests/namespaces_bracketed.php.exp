<?hh

namespace NS1 {
  const c = 0;
  function f(): void {
  }
  class C {
  }
  interface I {
  }
  trait T {
  }
}

namespace NS2 {
  use \NS1\C, \NS1\I, \NS1\T;
  class D extends C implements I {
    use T;
  }
  \NS1\f();

  use \NS1\C as C2;
  $c2 = new C2;
}

namespace {
  use \NS1\{C, I, T};
  use function \NS1\f;
  use const \NS1\c;
}

namespace {
  use function \NS1\{f};
  use const \NS1\{c};
}
