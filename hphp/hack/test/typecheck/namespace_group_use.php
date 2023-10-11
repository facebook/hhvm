<?hh

namespace Foo\Bar\Baz {
  function f1(): void {}
  function f2(): void {}
  const int CN = 1;
  class CL {}
  class CL2 {}
}

namespace X {
  use Foo\Bar\Baz\{CL, function f1, const CN, CL2};

  function f(): void {
    new CL();
    f1();
    \var_dump(CN);
    new CL2();
  }
}

namespace X2 {
  use function Foo\Bar\Baz\{f1, f2};

  function f(): void {
    f1();
    f2();
  }
}
