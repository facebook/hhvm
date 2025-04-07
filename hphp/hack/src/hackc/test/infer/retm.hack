// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// Random instruction tests.

// TEST-CHECK-BAL: define $root.multi_ret
// CHECK: define $root.multi_ret($this: *void, $a: .notnull *HackInt) : .notnull *HackInt {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   store &$a <- $builtins.hack_int(7): *HackMixed
// CHECK: // .column 3
// CHECK:   n0 = $builtins.hhbc_is_type_int($builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   n1 = $builtins.hhbc_verify_type_pred($builtins.hack_int(5), n0)
// CHECK: // .column 3
// CHECK:   n2: *HackMixed = load &$a
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK: // .column 3
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK: // .column 3
// CHECK:   n5 = $builtins.hhbc_new_vec($builtins.hack_int(5), n2)
// CHECK: // .column 3
// CHECK:   ret n5
// CHECK: }
function multi_ret(inout int $a): int {
  $a = 7;
  return 5;
}
