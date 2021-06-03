<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

enum class E : mixed {
  int A = 42;
}

function g(): int {
  $x = E#A;
  return E::valueOf($x);
}

<<__EntryPoint>>
function main(): void {
  echo g();
  echo "\n";
}
