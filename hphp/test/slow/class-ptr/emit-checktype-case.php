<?hh

class C {
  public static function f(): void { echo "bye\n"; }
}

<<__EntryPoint>>
function main(): void {
  $x = __hhvm_intrinsics\create_class_pointer("C");
  echo "hello\n";
  $x::f();

  $y = __hhvm_intrinsics\create_class_pointer("C");
  $y::f();
}
