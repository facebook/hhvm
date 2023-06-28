<?hh

namespace {
  function foo() :mixed{
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
