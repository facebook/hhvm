// RUN: %hackc compile-infer %s | FileCheck %s

// Random instruction tests.

// CHECK: define $root.multi_ret(this: *void, $a: *HackInt) : *HackInt {
// CHECK: #b0:
// CHECK:   store &$a <- $builtins.hack_int(7): *HackMixed
// CHECK:   n0 = $builtins.hhbc_is_type_int($builtins.hack_int(5))
// CHECK:   n1 = $builtins.hhbc_not(n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK:   n2 = $builtins.hhbc_verify_failed()
// CHECK:   unreachable
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK:   n3: *HackMixed = load &$a
// CHECK:   n4 = $builtins.hhbc_is_type_int(n3)
// CHECK:   n5 = $builtins.hhbc_not(n4)
// CHECK:   jmp b3, b4
// CHECK: #b3:
// CHECK:   prune $builtins.hack_is_true(n5)
// CHECK:   n6 = $builtins.hhbc_verify_failed()
// CHECK:   unreachable
// CHECK: #b4:
// CHECK:   prune ! $builtins.hack_is_true(n5)
// CHECK:   n7 = $builtins.hhbc_new_vec($builtins.hack_int(5), n3)
// CHECK:   ret n7
// CHECK: }
function multi_ret(inout int $a): int {
  $a = 7;
  return 5;
}
