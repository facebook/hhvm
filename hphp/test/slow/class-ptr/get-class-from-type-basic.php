<?hh

class C {
  public static function f(): void { echo "called f\n"; }
}
<<__EntryPoint>>
function main(): void {
  $c = HH\ReifiedGenerics\get_class_from_type<C>();
  $c::f();
}
