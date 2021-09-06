<?hh
<<file:__EnableUnstableFeatures('abstract_enum_class')>>

enum class E : mixed {
  int X = 0;
}

enum class F : mixed {
  int X = 1;
}

enum class G : mixed extends E, F {
}
