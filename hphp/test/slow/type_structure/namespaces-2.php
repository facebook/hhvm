<?hh

namespace Foo {
  type Bar<T> = T;
}
namespace {
  type Bar<T> = T;
  <<__EntryPoint>>
  function foo(): void {
    var_dump(42 as Bar<int>);
    var_dump(42 as Foo\Bar<int>);
  }
}
