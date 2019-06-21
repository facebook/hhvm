<?hh

namespace {
  function foo() {
    \var_dump(__NAMESPACE__);
  }
}
namespace B {
}
namespace B {
  <<__EntryPoint>> function main(): void {
    \foo();
  }
}
