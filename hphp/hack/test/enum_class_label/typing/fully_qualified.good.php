<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

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
