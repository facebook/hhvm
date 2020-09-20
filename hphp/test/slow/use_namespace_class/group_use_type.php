<?hh

namespace MyNS {
  class Foo {
    public function __construct() {
      \var_dump(__CLASS__);
    }
  }

  class Bar {
    public function __construct() {
      \var_dump(__CLASS__);
    }
  }

  class Herp {
    public function __construct() {
      \var_dump(__CLASS__);
    }
  }

  class Derp {
    public function __construct() {
      \var_dump(__CLASS__);
    }
  }
}
namespace Type {
  class Hello {
    public function __construct() {
      \var_dump(__CLASS__);
    }
  }
}

namespace {
  use type MyNS\{Foo, Bar};
  use type MyNS\{
    Herp,
    Derp,
  };

  <<__EntryPoint>> function main(): void {
    new Type\Hello();
    new Foo();
    new Bar();
    new Herp();
    new Derp();
  }
}
