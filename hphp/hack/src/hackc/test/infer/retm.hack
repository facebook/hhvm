// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// Random instruction tests.

// TEST-CHECK-BAL: define $root.multi_ret
// CHECK: define $root.multi_ret($this: *void, $a: .notnull *HackInt) : .notnull *HackInt {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   store &$a <- $builtins.hack_int(7): *HackMixed
// CHECK: // .column 3
// CHECK:   n0: *HackMixed = load &$a
// CHECK: // .column 3
// CHECK:   n1 = $builtins.hhbc_is_type_int(n0)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_new_vec($builtins.hack_int(5), n0)
// CHECK: // .column 3
// CHECK:   ret all n3
// CHECK: }
function multi_ret(inout int $a): int {
  $a = 7;
  return 5;
}

// TEST-CHECK-BAL: define $root.softReturn
// CHECK: define $root.softReturn($this: *void) : *HackMixed {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   ret all $builtins.hack_string("hello")
// CHECK: }
function softReturn(): <<__Soft>> string {
  return "hello";
}
