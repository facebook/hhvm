<?hh

namespace Dict\Foo\Bar {
  function foo(): void {
    echo __FUNCTION__;
  }
}

namespace HH\Lib\Dict\Foo\Bar {
  function foo(): void {
    echo __FUNCTION__;
  }
}

namespace {
  use namespace Dict\Foo\Bar;

  <<__EntryPoint>> function main(): void {
  Bar\foo();
  }
}
