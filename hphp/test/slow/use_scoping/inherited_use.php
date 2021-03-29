<?hh

namespace A {
  function foo(): void { \var_dump('hello!'); }
}

use function A\foo;

namespace B {
  <<__EntryPoint>>
  function main(): void {
    foo();
  }
}
