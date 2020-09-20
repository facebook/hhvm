<?hh

namespace A {
  const B = 'B';
}

namespace C {
  <<__EntryPoint>> function main(): void {
    $name = '\\\\A\B';
    \var_dump(\constant($name));
  }
}
