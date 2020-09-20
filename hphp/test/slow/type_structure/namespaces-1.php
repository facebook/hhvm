<?hh

namespace Foo {
  type Bar<T> = T;
}
namespace {
  type Bar<T> = T;
  <<__EntryPoint>>
  function foo(): void {
    var_dump(type_structure('Bar'));
    var_dump(type_structure('Foo\\Bar'));
  }
}
