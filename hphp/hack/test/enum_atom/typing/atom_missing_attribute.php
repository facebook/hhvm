<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

enum class E : int {
  int A = 42;
}

// param doesn't have <<_Atom>>
function f(HH\MemberOf<E, int> $z) : int {
  return $z;
}

<<__EntryPoint>>
function main(): void {
  f(#A); // f missing <<_Atom>> attribute for param
}
