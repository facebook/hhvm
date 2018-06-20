<?hh

namespace HH\Lib\Dict {
  function foo(): void {
    echo __FUNCTION__."\n";
  }
}

namespace Main {
  Dict\foo(); // ok
  \Dict\foo(); // ok TODO(T22617428) should be error
  \HH\Lib\Dict\foo(); // ok
  HH\Lib\Dict\foo(); // error
}
