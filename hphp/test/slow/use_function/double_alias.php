<?hh

namespace foo {
  function bar() {
    return __FUNCTION__;
  }
}

namespace {
  use function foo\bar;
  use function foo\bar as bar;

  <<__EntryPoint>> function main(): void {
    bar();
  }
}
