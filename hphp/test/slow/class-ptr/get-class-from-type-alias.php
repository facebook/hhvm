<?hh

class C {
  public static function f(): void { echo "called f\n"; }
}
type T = C;

<<__EntryPoint>>
function main(): void {
  $c = HH\ReifiedGenerics\get_class_from_type<T>();
  $c::f();
}
