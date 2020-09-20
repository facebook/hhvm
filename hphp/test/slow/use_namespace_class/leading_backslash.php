<?hh

namespace MyNS {
  class Foo {
    public function __construct() {
      \var_dump(__CLASS__);
    }
  }
}

namespace {
  use type \MyNS\Foo;
  use namespace \MyNS as FooNS;
  <<__EntryPoint>> function main(): void {
  new Foo();
  new FooNS\Foo();
  }
}
