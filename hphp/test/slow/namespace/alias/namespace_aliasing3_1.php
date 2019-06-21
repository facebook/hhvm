<?hh

namespace HH\Lib\Dict {
  function foo(): void {
    echo __FUNCTION__."\n";
  }
}

namespace Main {
  <<__EntryPoint>> function main(): void {
  Dict\foo(); // ok
  \Dict\foo(); // error
  }
}
