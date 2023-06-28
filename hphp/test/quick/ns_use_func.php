<?hh
namespace A {
  function f() :mixed{ return 'a'; }
}
namespace B {
  use A\f;
  <<__EntryPoint>> function main(): void { \var_dump(f()); }
}
