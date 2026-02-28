<?hh

namespace A {
  function f() :mixed{ return 'A/f'; }
}

namespace C {
  <<__EntryPoint>> function main(): void {
    $name = '\\\\A\f';
    \var_dump($name());
  }
}
