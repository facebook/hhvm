<?hh
// alias C to D and A to B
namespace D {
  function x() : void {
    echo "D\n";
  }
}
namespace C {
  function x() : void {
    echo "C\n";
  }
}


namespace A {
  function x() : void {
    echo "A\n";
  }
}

namespace B {
  function x() : void {
    echo "B\n";
  }
}

namespace {
  use A; // overrides alias
  use namespace C; // overrides alias
  <<__EntryPoint>> function main(): void {
  A\x(); // A\x();
  C\x(); // C\x();
  }
}
