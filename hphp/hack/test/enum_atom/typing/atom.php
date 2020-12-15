<?hh
<<file:__EnableUnstableFeatures('enum_atom', 'enum_class')>>

enum E : int as int {
  A = 42;
  B = 1664;
}

// Not allowed on enums at the moment
function f(<<__Atom>> E $x): int {
  return $x;
}

interface I {}
class Box implements I {
  public function __construct(public int $x) {}
}

enum class EE : I {
  A<Box>(new Box(42));
}

function ff(<<__Atom>> HH\EnumMember<EE, Box> $x) : int {
  return $x->data()->x;
}

<<__EntryPoint>>
function main(): void {
  ff(#A);
  ff(#C); // unknown constant
}

class C {}
function wrong_upper_bound<reify T as C>(
    <<__Atom>> HH\EnumMember<T, Box> $x
  ): mixed {
  return $x->data()->x;
}
