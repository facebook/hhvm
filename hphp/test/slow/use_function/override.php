<?hh

namespace foo {
  function bar() :mixed{
    return __FUNCTION__;
  }
}

namespace {
  function bar() :mixed{
    return __FUNCTION__;
  }
  use function foo\bar as bar;

  <<__EntryPoint>>
  function main() :mixed{}
}
