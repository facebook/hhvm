<?hh

class C {
  public static function f(): void { echo "called f\n"; }
}
abstract class DAbs {
  abstract const type T;
  public static function call(): void {
    $c = HH\ReifiedGenerics\get_class_from_type<this::T>();
    $c::f();
  }
}
class D extends DAbs {
  const type T = C;
}
<<__EntryPoint>>
function main(): void {
  D::call();
  DAbs::call();
}
