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
  <<__EntryPoint>> function main(): void {
  Dict\foo();
  \Dict\foo();
  }
}
