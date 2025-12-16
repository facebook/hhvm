// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static extends HH::classname = .kind="class" .static {
// CHECK:   a: .public *HackInt
// CHECK: }

// TEST-CHECK-BAL: define C$static.foo
// CHECK: define C$static.foo($this: .notnull *C$static) : *void {
// CHECK: #b0:
// CHECK: // .column 8
// CHECK:   n0 = __sil_lazy_class_initialize(<C>)
// CHECK: // .column 5
// CHECK:   store n0.?.a <- $builtins.hack_int(6): *HackMixed
// CHECK: // .column 4
// CHECK:   ret null
// CHECK: }

class C {
  public static int $a = 5;

  public static function foo(): void {
    C::$a = 6;
  }
}
