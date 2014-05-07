<?hh

namespace test;

class Foo {
  function foo() : this {
    return $this;
  }
}

$foo = new Foo();
var_dump($foo->foo());
