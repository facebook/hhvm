<?hh

namespace N1 {
  function foo(): void {}
  const int foo = 1;
}

namespace N2 {
  use N1\{function foo, function foo};
}
