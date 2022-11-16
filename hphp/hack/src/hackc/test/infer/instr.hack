// RUN: %hackc compile-infer %s | FileCheck %s

// Random instruction tests.

// CHECK: define $root.binops(this: *void, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
function binops(int $a, int $b): void {
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_add(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a + $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_eq(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a == $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_gt(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a > $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_gte(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a >= $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_lt(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a < $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_lte(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a <= $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_nsame(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a !== $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_neq(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a != $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_cmp_same(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a === $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_concat(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a . $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_div(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a / $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_modulo(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a % $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_mul(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a * $b;
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_sub(n{{[0-9]+}}, n{{[0-9]+}})
  $c = $a - $b;
}

// CHECK: define $root.unops(this: *void, $a: *HackMixed) : *HackMixed {
function unops(int $a): void {
  // CHECK:  n{{[0-9]+}} = $builtins.hhbc_not(n{{[0-9]+}})
  $c = ! $a;
}
