// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static = .kind="class" .static {
// CHECK:   a: .public *HackInt
// CHECK: }

// TEST-CHECK-BAL: define C$static.foo
// CHECK: define C$static.foo($this: *C$static) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_class_get_c($builtins.hack_string("C"))
// CHECK:   n1 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("a"), $builtins.hack_int(6))
// CHECK:   ret null
// CHECK: }

class C {
  public static int $a = 5;

  public static function foo(): void {
    C::$a = 6;
  }
}
