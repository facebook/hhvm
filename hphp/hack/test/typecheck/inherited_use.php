<?hh

namespace A {
  function foo(): void {}
}

use function A\foo;

namespace B {
  <<__EntryPoint>>
  function main(): void {
    foo();
  }
}
