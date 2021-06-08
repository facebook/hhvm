<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

enum class E : mixed {
  int A = 42;
}

function bad(): void {
  $x = E#B;
  $y = F#A;
}
