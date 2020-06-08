<?hh

namespace foo {
  function bar() {
    return __FUNCTION__;
  }
}

namespace {
  function bar() {
    return __FUNCTION__;
  }
  use function foo\bar as bar;

  <<__EntryPoint>>
  function main() {}
}
