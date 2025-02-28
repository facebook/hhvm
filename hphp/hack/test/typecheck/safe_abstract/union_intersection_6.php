<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

abstract class Abs1 {
  public static abstract function foo(): void;
}
abstract class Abs2 {
  public static abstract function foo(): void;
 }
final class C1 {
  public static function foo(): void {}
}


function make(): (classname<Abs1> | classname<Abs2> | concreteclassname<C1>) {
  return make();
}

<<__EntryPoint>>
function main(): void {
  $class = make();
  // Unsafe call: foo::Abs1 might not exist because the receiver might be abstract.
  // Unsafe call: foo::Abs2 might not exist because the receiver might be abstract.
  $class::foo();
}
