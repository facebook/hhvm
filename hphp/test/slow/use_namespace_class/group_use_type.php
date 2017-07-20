<?hh

namespace MyNS {
  class Foo {
    public function __construct() {
      var_dump(__CLASS__);
    }
  }

  class Bar {
    public function __construct() {
      var_dump(__CLASS__);
    }
  }
}

namespace {
  use type MyNS\{Foo, Bar};
  new Foo();
  new Bar();
}
