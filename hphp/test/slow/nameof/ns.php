<?hh

namespace B {
  <<__EntryPoint>>
  function main(): void {
    \var_dump(C::class);
    \var_dump(nameof C);
    \var_dump(\C::class);
    \var_dump(nameof \C);
    \var_dump(A\C::class);
    \var_dump(nameof A\C);
    \var_dump(\A\C::class);
    \var_dump(nameof \A\C);
  }
}
