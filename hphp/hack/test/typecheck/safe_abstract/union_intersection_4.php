<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>
<<file: __EnableUnstableFeatures('like_type_hints')>>

<<__ConsistentConstruct>>
abstract class Abs1 {}


function make(): (string & ~classname<Abs1>) {
  return make();
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  // Unsafe use of `new`: `Abs1` might be abstract (Warn[12015])
  new $class();
}
