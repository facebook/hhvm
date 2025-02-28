<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

<<__ConsistentConstruct>>
abstract class Abs1 {}
<<__ConsistentConstruct>>
abstract class Abs2 {}


function make(): (classname<Abs1> & classname<Abs2>) {
  return make();
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  // Unsafe use of new: Abs1 might be abstract
  new $class();
}
