<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

<<__ConsistentConstruct>>
interface I1 {}
<<__ConsistentConstruct>>
interface I2 {}


function make(): (classname<I1> & classname<I2>) {
  while (true) {

  }
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  // Unsafe use of new: I1 might be abstract
  new $class();
}
