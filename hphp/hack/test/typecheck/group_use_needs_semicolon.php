<?hh // strict

namespace Foo\Bar\Baz {
  function f1(): void {}
  function f2(): void {}
  const int CN = 1;
  class CL {}
  class CL2 {}
}

namespace Y {
  use Foo\Bar\Baz\{CL, function f1, const CN, CL2};

  function foo(): void {}
}

namespace X {
  use Foo\Bar\Baz\{CL, function f1, const CN, CL2}

  function foo(): void {}
}
