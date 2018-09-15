<?hh // strict

namespace herp {
  class derp {}
}

namespace herp\derp {
  function foo(): void {}
}

namespace {
  use namespace herp\derp;

  function main(): void {
    derp\foo();
    new derp();
  }
}
