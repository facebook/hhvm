<?hh
namespace {
  function foo(): void { \var_dump(__FUNCTION__); }
}

namespace A {
  function foo(): void { \var_dump(__FUNCTION__); }
}

namespace {
  <<__EntryPoint>>
  function main(): void {
    foo();
    A\foo();
  }
}
