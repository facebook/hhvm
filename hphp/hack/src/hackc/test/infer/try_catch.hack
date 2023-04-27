// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: local $e: *void, $x: *void
// CHECK: #b0:
// CHECK:   n0 = $root.a(null, $builtins.hack_int(0))
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1 = $root.a(null, $builtins.hack_int(1))
// CHECK:   store &$x <- n1: *HackMixed
// CHECK:   jmp b5
// CHECK:   .handlers b2
// CHECK: #b2(n2: *HackMixed):
// CHECK:   n3 = $builtins.hack_is_type(n2, $builtins.hack_string("Exception"))
// CHECK:   jmp b3, b4
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK:   n4 = $builtins.hhbc_throw(n2)
// CHECK:   unreachable
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK:   store &$e <- n2: *HackMixed
// CHECK:   n5 = $root.a(null, $builtins.hack_int(2))
// CHECK:   jmp b5
// CHECK: #b5:
// CHECK:   n6 = $root.a(null, $builtins.hack_int(3))
// CHECK:   ret null
// CHECK: }
function main(): void {
  a(0);
  try {
    $x = a(1);
  } catch (Exception $e) {
    a(2);
  }
  a(3);
}

function a(int $x): int { return $x + 1; }
