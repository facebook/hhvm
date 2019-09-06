<?hh

namespace A {
  <<__DynamicallyConstructible>> class B {}
}

namespace C {
  <<__EntryPoint>> function main(): void {
    $name = '\\\\A\B';
    \var_dump(new $name);
  }
}
