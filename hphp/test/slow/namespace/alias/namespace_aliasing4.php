<?hh

namespace Dict {
  function foo(): void {
    echo "Dict\n";
  }
}

namespace HH\Lib\Dict {
  function foo(): void {
    echo "HH\Lib\Dict\n";
  }
}

namespace {
  Dict\foo();
  \Dict\foo();
}
