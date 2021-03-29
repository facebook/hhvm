<?hh
namespace A {
  function foo(): void {}
}
namespace B {
  function foo(): void {}
}
namespace C {
  use function A\foo;
  use function B\foo;
}
