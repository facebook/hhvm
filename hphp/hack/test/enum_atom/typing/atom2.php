<?hh
<<file:__EnableUnstableFeatures('enum_atom', 'enum_class')>>

interface I {}
class C implements I {}

enum class E : I {
  A<C>(new C());
}

function show(HH\Elt<E, C> $enum) : void {
  echo $enum->name();
}

function show_atom(<<__Atom>>HH\Elt<E, C> $enum): void {
  echo $enum->name();
}

<<__EntryPoint>>
function main(): void {
  show(E::A);
  show_atom(#A);
  show_atom(E::A); // invalid
  show(#A); // invalidd
}
