<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

<<__ConsistentConstruct>>
abstract class Abs1 {}
<<__ConsistentConstruct>>
abstract class Abs2 {}
<<__ConsistentConstruct>>
final class C1 {}

function make(): (class<Abs1> | class<Abs2> | concrete<class<C1>>) {
  while (true) {
  }
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  // Unsafe use of new: Abs1 might be abstract
  // Unsafe use of new: Abs2 might be abstract
  new $class();
}
