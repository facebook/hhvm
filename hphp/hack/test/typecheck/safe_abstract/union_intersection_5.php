<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>
<<file: __EnableUnstableFeatures('like_type_hints')>>

final class C1 {}
final class C2 {}

function make(): (concrete<classname<C1>> | concrete<classname<C2>>) {
  while (true) {
  }
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  new $class();
}
