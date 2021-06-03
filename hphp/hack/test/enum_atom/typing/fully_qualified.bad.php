<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

enum class E : mixed {
  int A = 42;
}

function bad(): void {
  $x = E#B;
  $y = F#A;
}
