<?hh

namespace HH\Lib\Dict {
  function foo(): void {
    echo __FUNCTION__."\n";
  }
}

namespace {
  <<__EntryPoint>> function main(): void {
  HH\Lib\Dict\foo(); // ok
  \HH\Lib\Dict\foo(); // ok
  Dict\foo(); // ok
  \Dict\foo(); // error
  }
}
