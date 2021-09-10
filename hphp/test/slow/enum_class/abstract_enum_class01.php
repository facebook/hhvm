<?hh
<<file:__EnableUnstableFeatures('abstract_enum_class')>>

abstract enum class E : mixed {
  abstract int X;
}

// missing constant initialization
enum class H : mixed extends E {
}
