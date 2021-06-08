<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

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

function show_via_label(<<__ViaLabel>>HH\MemberOf<E, C> $enum): void {
  echo $enum->name;
}

<<__EntryPoint>>
function main(): void {
  show(E::A);
  show_via_label(#A);
  show_via_label(E::A); // invalid
  show(#A); // invalidd
}
