<?hh

namespace {
  <<__DynamicallyCallable>>
  function foo() {
    \var_dump(__NAMESPACE__);
  }
}
namespace B {
  <<__DynamicallyCallable>>
  function foo() {
    \var_dump(__NAMESPACE__);
  }
}
namespace B {
  <<__EntryPoint>> function main(): void {
    $a = 'foo';
    $a();
  }
}
