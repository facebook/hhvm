<?hh
<<file:__EnableUnstableFeatures('abstract_enum_class')>>

abstract enum class E : mixed {
  abstract int X;
}

abstract enum class F : mixed {
  abstract int X;
}

enum class G : mixed extends E, F {
  int X = 42;
}

<<__EntryPoint>>
function main(): void {
  echo G::X;
  echo "\n";
}
