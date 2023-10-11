<?hh

namespace Foo {
  class Derp {
    public function __construct(int $_) {
      \var_dump(__LINE__);
    }
  }
}

namespace Bar {
  function Derp(string $_): void {
    \var_dump(__LINE__);
  }
}

namespace Baz {
  function Derp(float $_): void {
    \var_dump(__LINE__);
  }
}

namespace {
  use type Foo\Derp;
  use function Bar\Derp;
  use namespace Baz as Derp;

  function main(): void {
    // This is a test, but it still makes me cry :'(
    new Derp(123); // \Foo\Derp
    Derp('123'); // \Bar\Derp
    Derp\Derp(1.23); // \Baz\Derp
  }
}
