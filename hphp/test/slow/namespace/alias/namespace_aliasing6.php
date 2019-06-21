<?hh

namespace HH\Lib\Dict {
  function foo(): void {
    echo __FUNCTION__;
  }
}

namespace Some\Other\Dict {
  function foo(): void {
    echo __FUNCTION__;
  }
}

namespace {
  use namespace Some\Other\Dict;

  <<__EntryPoint>> function main(): void {
  Dict\foo();
  }
}
