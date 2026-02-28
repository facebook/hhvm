<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

class C {
  const int X = "hello" as ~int;
}
