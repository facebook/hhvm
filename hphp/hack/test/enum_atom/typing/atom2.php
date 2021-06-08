<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

interface I {}
class C implements I {
  public function __construct(public string $name)[] {}
}

enum class E : I {
   C A = new C("A");
}

function show(HH\MemberOf<E, C> $enum) : void {
  echo $enum->name;
}

function show_atom(<<__ViaLabel>>HH\MemberOf<E, C> $enum): void {
  echo $enum->name;
}

<<__EntryPoint>>
function main(): void {
  show(E::A);
  show_atom(#A);
  show_atom(E::A); // invalid
  show(#A); // invalidd
}
