<?hh

namespace test;

class Foo {
  function foo() : this {
    return $this;
  }
}


<<__EntryPoint>>
function main_namespace_this() {
$foo = new Foo();
\var_dump($foo->foo());
}
