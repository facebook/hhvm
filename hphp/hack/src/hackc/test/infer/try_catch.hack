// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: local $e: *void, $x: *void
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $root.a(null, $builtins.hack_int(0))
// CHECK: // .column 3
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK: // .column 10
// CHECK:   n1 = $root.a(null, $builtins.hack_int(1))
// CHECK: // .column 5
// CHECK:   store &$x <- n1: *HackMixed
// CHECK: // .column 3
// CHECK:   jmp b5
// CHECK:   .handlers b2
// CHECK: #b2(n2: *HackMixed):
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hack_bool(__sil_instanceof(n2, <Exception>, 0))
// CHECK: // .column 3
// CHECK:   jmp b3, b4
// CHECK: #b3:
// CHECK: // .column 3
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK: // .column 3
// CHECK:   throw n2
// CHECK: #b4:
// CHECK: // .column 3
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK: // .column 3
// CHECK:   store &$e <- n2: *HackMixed
// CHECK: // .column 5
// CHECK:   n4 = $root.a(null, $builtins.hack_int(2))
// CHECK: // .column 3
// CHECK:   jmp b5
// CHECK: #b5:
// CHECK: // .column 3
// CHECK:   n5 = $root.a(null, $builtins.hack_int(3))
// CHECK: // .column 2
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

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main2($this: *void) : *void {
// CHECK: local $e: *void, $x: *void
// CHECK: #b0:
// CHECK: // .column 10
// CHECK:   n0 = $root.a(null, $builtins.hack_int(1))
// CHECK: // .column 5
// CHECK:   store &$x <- n0: *HackMixed
// CHECK: // .column 3
// CHECK:   jmp b4
// CHECK:   .handlers b1
// CHECK: #b1(n1: *HackMixed):
// CHECK: // .column 3
// CHECK:   n2 = $builtins.hack_bool(__sil_instanceof(n1, <Exception>, 0))
// CHECK: // .column 3
// CHECK:   jmp b2, b3
// CHECK: #b2:
// CHECK: // .column 3
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK: // .column 3
// CHECK:   throw n1
// CHECK: #b3:
// CHECK: // .column 3
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK: // .column 3
// CHECK:   store &$e <- n1: *HackMixed
// CHECK: // .column 5
// CHECK:   n3 = $root.a(null, $builtins.hack_int(2))
// CHECK: // .column 3
// CHECK:   jmp b4
// CHECK: #b4:
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function main2(): void {
  try {
    $x = a(1);
  } catch (Exception $e) {
    a(2);
  }
}

function a(int $x): int { return $x + 1; }
