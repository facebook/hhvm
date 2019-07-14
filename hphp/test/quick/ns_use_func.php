<?hh
namespace A {
  function f() { return 'a'; }
}
namespace B {
  use A\f;
  <<__EntryPoint>> function main(): void { \var_dump(f()); }
}
