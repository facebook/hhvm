<?hh
<<file:__EnableUnstableFeatures('enum_class_label', 'abstract_enum_class')>>

abstract enum class E : mixed {
  abstract int X;
  int Y = 53;
}

enum class F : mixed extends E {
  int X = 42;
}

function good() : void {
  echo E::nameOf(E#X);
  echo "\n";
  echo E::nameOf(E#Y);
  echo "\n";
  echo F::nameOf(F#X);
  echo "\n";
  echo F::nameOf(F#Y);
  echo "\n";
  echo F::valueOf(F#X);
  echo "\n";
  echo F::valueOf(F#Y);
  echo "\n";
}

function bad() : void {
  echo E::valueOf(E#X);
  echo "\n";
  echo E::valueOf(E#Y);
  echo "\n";
}
