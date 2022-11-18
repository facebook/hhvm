// RUN: %hackc compile-infer %s | FileCheck %s

// Random instruction tests.

// CHECK: define $root.multi_ret(this: *void, $a: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   store &$a <- $builtins.hack_int(7): *HackMixed
// CHECK:   n0 = $builtins.hhbc_is_type_int($builtins.hack_int(5))
// CHECK:   n1 = $builtins.hhbc_verify_type_pred($builtins.hack_int(5), n0)
// CHECK:   n2: *HackMixed = load &$a
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK:   n4 = $builtins.hhbc_new_vec(n1, n2)
// CHECK:   ret n4
// CHECK: }
function multi_ret(inout int $a): int {
  $a = 7;
  return 5;
}
