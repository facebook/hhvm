<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

enum class E : mixed {
  int A = 42;
}

function good(): int {
  $x = E#A;
  return E::valueOf($x);
}

<<__EntryPoint>>
function main(): void {
  echo good();
  echo "\n";
}
