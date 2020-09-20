<?hh

namespace HH\SomethingElse\Dict {
  function foo(): void {
    echo __FUNCTION__."\n";
  }
}

namespace {
  <<__EntryPoint>> function main(): void {
  HH\SomethingElse\Dict\foo(); // ok
  \HH\SomethingElse\Dict\foo(); // ok
  Dict\foo(); // error
  }
}
