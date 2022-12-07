// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *HackMixed {
// CHECK: local $e: *void, $x: *void, base: *HackMixed
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("Exception")
// CHECK:   n1 = $root.a(null, $builtins.hack_int(0))
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n2 = $root.a(null, $builtins.hack_int(1))
// CHECK:   store &$x <- n2: *HackMixed
// CHECK:   jmp b5
// CHECK:   .handlers b2
// CHECK: #b2(n3: *HackMixed):
// CHECK:   n4 = $builtins.hack_is_type(n3, n0)
// CHECK:   jmp b3, b4
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   n5 = $builtins.hhbc_throw(n3)
// CHECK:   unreachable
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   store &$e <- n3: *HackMixed
// CHECK:   n6 = $root.a(null, $builtins.hack_int(2))
// CHECK:   jmp b5
// CHECK: #b5:
// CHECK:   n7 = $root.a(null, $builtins.hack_int(3))
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
