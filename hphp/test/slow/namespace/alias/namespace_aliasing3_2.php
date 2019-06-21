<?hh

namespace HH\Lib\Dict {
  function foo(): void {
    echo __FUNCTION__."\n";
  }
}

namespace Main {
  <<__EntryPoint>> function main(): void {
  \HH\Lib\Dict\foo(); // ok
  HH\Lib\Dict\foo(); // error
  }
}
