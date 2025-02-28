<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

<<__ConsistentConstruct>>
abstract class Abs1 {}
<<__ConsistentConstruct>>
abstract class Abs2 { }
<<__ConsistentConstruct>>
final class C1 { }


function make(): (classname<Abs1> | classname<Abs2> | concreteclassname<C1>) {
  return C1::class;
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  // Unsafe use of new: Abs1 might be abstract
  // Unsafe use of new: Abs2 might be abstract
  new $class();
}
