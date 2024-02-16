// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static = .kind="class" .static {
// CHECK:   a: .public *HackInt
// CHECK: }

// TEST-CHECK-BAL: define C$static.foo
// CHECK: define C$static.foo($this: *C$static) : *void {
// CHECK: #b0:
// CHECK:   n0: *C$static = load &$this
// CHECK:   store n0.?.a <- $builtins.hack_int(6): *HackMixed
// CHECK:   ret null
// CHECK: }

class C {
  public static int $a = 5;

  public static function foo(): void {
    C::$a = 6;
  }
}
