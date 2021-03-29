<?hh

namespace A {
  function foo(): void {
    \var_dump(__FUNCTION__);
  }
}

namespace B {
  function foo(): void {
    \var_dump(__FUNCTION__);
  }
}

use function A\foo;

namespace C {
  use function B\foo;
  <<__EntryPoint>>
  function main(): void {
      foo();
  }
}
