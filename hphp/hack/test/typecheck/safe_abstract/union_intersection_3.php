<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

<<__ConsistentConstruct>>
abstract class Abs1 {}
<<__ConsistentConstruct>>
abstract class Abs2 {}

type Classes = (Abs1 & Abs2);

function make(): classname<Classes> {
  while (true) {

  }
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  new $class();
}
