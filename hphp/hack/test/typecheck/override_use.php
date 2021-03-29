<?hh

namespace A {
  function foo(): void {}
}

use function A\foo;

namespace B {
  use function C\foo;
}
