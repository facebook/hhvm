<?hh

namespace {
  <<__DynamicallyCallable>> function foo() :mixed{
    \var_dump(__NAMESPACE__);
  }
}
namespace B {
  <<__DynamicallyCallable>> function foo() :mixed{
    \var_dump(__NAMESPACE__);
  }
}
namespace B {
  <<__EntryPoint>> function main(): void {
    \call_user_func(\HH\dynamic_fun('foo'));
  }
}
