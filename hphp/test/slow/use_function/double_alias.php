<?hh

namespace foo {
  function bar() :mixed{
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
