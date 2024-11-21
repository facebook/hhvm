<?hh

class C {
  public static function f(): void { echo "called f\n"; }
}
function call<reify T>(): void {
  $c = HH\ReifiedGenerics\get_class_from_type<T>();
  $c::f();
}
function call2<reify T2>(): void {
  call<T2>();
}
abstract class DAbs {
  abstract const type T;
  public static function meth(): void {
    call<this::T>();
  }
}
class D extends DAbs {
  const type T = C;
}
<<__EntryPoint>>
function main(): void {
  call<C>();
  D::meth();
  call2<C>();
  call<E>(); // unbound name
}
