<?hh

namespace HH\Lib\Dict {
  function foo(): void {
    echo __FUNCTION__."\n";
  }
}

namespace Main {
  Dict\foo(); // ok
  \Dict\foo(); // error
}
