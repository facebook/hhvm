<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

class C {
  const type T = ~int;
}

<<__EntryPoint>>
function main(): void {
  3 as C::T;
  "hello" as C::T;
}
