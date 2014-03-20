<?hh

namespace TestNS {
  class C {}

  function f() {
    return new C();
  }

  function g() {
    return new \TestNS\C();
  }
}

namespace TestNS2 {
  class D {}

  function f() {
    return new D();
  }

  function g() {
    return new \TestNS\C();
  }
}

namespace TestNS3 {
  use \TestNS\C;

  function f() {
    return new C();
  }
}

namespace Foo\Bar {
  class C {}
}

namespace Foo {
  function f() {
    return new Bar\C();
  }
}
