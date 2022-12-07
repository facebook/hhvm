// RUN: %hackc compile-infer %s | FileCheck %s

// Random instruction tests.

// TEST-CHECK-1: define $root.binops
// CHECK: define $root.binops($this: *void, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
function binops(int $a, int $b): void {
  // TEST-CHECK-1*: $builtins.hhbc_add
  // CHECK:   n2 = $builtins.hhbc_add(n1, n0)
  $c = $a + $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_eq
  // CHECK:   n5 = $builtins.hhbc_cmp_eq(n4, n3)
  $c = $a == $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_gt
  // CHECK:   n8 = $builtins.hhbc_cmp_gt(n7, n6)
  $c = $a > $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_gte
  // CHECK:   n11 = $builtins.hhbc_cmp_gte(n10, n9)
  $c = $a >= $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_lt
  // CHECK:   n14 = $builtins.hhbc_cmp_lt(n13, n12)
  $c = $a < $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_lte
  // CHECK:   n17 = $builtins.hhbc_cmp_lte(n16, n15)
  $c = $a <= $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_nsame
  // CHECK:   n20 = $builtins.hhbc_cmp_nsame(n19, n18)
  $c = $a !== $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_neq
  // CHECK:   n23 = $builtins.hhbc_cmp_neq(n22, n21)
  $c = $a != $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_same
  // CHECK:   n26 = $builtins.hhbc_cmp_same(n25, n24)
  $c = $a === $b;
  // TEST-CHECK-1*: $builtins.hhbc_concat
  // CHECK:   n29 = $builtins.hhbc_concat(n28, n27)
  $c = $a . $b;
  // TEST-CHECK-1*: $builtins.hhbc_div
  // CHECK:   n32 = $builtins.hhbc_div(n31, n30)
  $c = $a / $b;
  // TEST-CHECK-1*: $builtins.hhbc_modulo
  // CHECK:   n35 = $builtins.hhbc_modulo(n34, n33)
  $c = $a % $b;
  // TEST-CHECK-1*: $builtins.hhbc_mul
  // CHECK:   n38 = $builtins.hhbc_mul(n37, n36)
  $c = $a * $b;
  // TEST-CHECK-1*: $builtins.hhbc_sub
  // CHECK:   n41 = $builtins.hhbc_sub(n40, n39)
  $c = $a - $b;
}

// TEST-CHECK-1: define $root.unops
// CHECK: define $root.unops($this: *void, $a: *HackMixed) : *HackMixed {
function unops(int $a): void {
  // TEST-CHECK-1*: $builtins.hhbc_not
  // CHECK:   n1 = $builtins.hhbc_not(n0)
  $c = ! $a;
}

// TEST-CHECK-BAL: define $root.check_shape
// CHECK: define $root.check_shape($this: *void) : *HackMixed {
// CHECK: local base: *HackMixed
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("a")
// CHECK:   n1 = $builtins.hack_string("b")
// CHECK:   n2 = $root.a(null)
// CHECK:   n3 = $root.b(null)
// CHECK:   n4 = $builtins.hack_new_dict(n0, n2, n1, n3)
// CHECK:   n5 = $root.f(null, n4)
// CHECK:   ret $builtins.hack_null()
// CHECK: }
function check_shape(): void {
  f(shape('a' => a(), 'b' => b()));
}
