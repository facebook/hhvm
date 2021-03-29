<?hh
namespace A {
  function foo(): void {}
}
namespace B {
  use function A\foo;
}
// Intentionally repeat: should be scoped the the block, not the NS
namespace B {
  use function A\foo;
}
namespace C {
  use function A\foo;
}
