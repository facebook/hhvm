<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>
<<file: __EnableUnstableFeatures('like_type_hints')>>

final class C1 {}
final class C2 {}


function make(): (concreteclassname<C1> | concreteclassname<C2>) {
  return make();
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  new $class();
}
