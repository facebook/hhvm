<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

enum class E : int {
  int A = 42;
}

// param doesn't have <<__ViaLabel>>
function f(HH\MemberOf<E, int> $z) : int {
  return $z;
}

<<__EntryPoint>>
function main(): void {
  f(#A); // f missing <<__ViaLabel>> attribute for param
}
