<?hh

namespace {
  function foo() {
    \var_dump(__NAMESPACE__);
  }
}
namespace B {
  function foo() {
    \var_dump(__NAMESPACE__);
  }
}
namespace B {
  <<__EntryPoint>> function main(): void {
    \call_user_func(fun('foo'));
  }
}
