// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// Random instruction tests.

// TEST-CHECK-1: define $root.binops
// CHECK: define $root.binops($this: *void, $a: *HackInt, $b: *HackInt) : *void {
function binops(int $a, int $b): void {
  // TEST-CHECK-1*: $builtins.hhbc_add
  // CHECK:   n2 = $builtins.hhbc_add(n1, n0)
  $c = $a + $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_eq
  // CHECK:   n3 = $builtins.hhbc_cmp_eq(n1, n0)
  $c = $a == $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_gt
  // CHECK:   n4 = $builtins.hhbc_cmp_gt(n1, n0)
  $c = $a > $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_gte
  // CHECK:   n5 = $builtins.hhbc_cmp_gte(n1, n0)
  $c = $a >= $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_lt
  // CHECK:   n6 = $builtins.hhbc_cmp_lt(n1, n0)
  $c = $a < $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_lte
  // CHECK:   n7 = $builtins.hhbc_cmp_lte(n1, n0)
  $c = $a <= $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_nsame
  // CHECK:   n8 = $builtins.hhbc_cmp_nsame(n1, n0)
  $c = $a !== $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_neq
  // CHECK:   n9 = $builtins.hhbc_cmp_neq(n1, n0)
  $c = $a != $b;
  // TEST-CHECK-1*: $builtins.hhbc_cmp_same
  // CHECK:   n10 = $builtins.hhbc_cmp_same(n1, n0)
  $c = $a === $b;
  // TEST-CHECK-1*: $builtins.hhbc_concat
  // CHECK:   n11 = $builtins.hhbc_concat(n1, n0)
  $c = $a . $b;
  // TEST-CHECK-1*: $builtins.hhbc_div
  // CHECK:   n12 = $builtins.hhbc_div(n1, n0)
  $c = $a / $b;
  // TEST-CHECK-1*: $builtins.hhbc_modulo
  // CHECK:   n13 = $builtins.hhbc_modulo(n1, n0)
  $c = $a % $b;
  // TEST-CHECK-1*: $builtins.hhbc_mul
  // CHECK:   n14 = $builtins.hhbc_mul(n1, n0)
  $c = $a * $b;
  // TEST-CHECK-1*: $builtins.hhbc_sub
  // CHECK:   n15 = $builtins.hhbc_sub(n1, n0)
  $c = $a - $b;
}

// TEST-CHECK-1: define $root.unops
// CHECK: define $root.unops($this: *void, $a: *HackInt) : *void {
function unops(int $a): void {
  // TEST-CHECK-1*: $builtins.hhbc_not
  // CHECK:   n1 = $builtins.hhbc_not(n0)
  $c = ! $a;
}

// TEST-CHECK-BAL: define $root.check_shape
// CHECK: define $root.check_shape($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.a(null)
// CHECK:   n1 = $root.b(null)
// CHECK:   n2 = $builtins.hack_new_dict($builtins.hack_string("a"), n0, $builtins.hack_string("b"), n1)
// CHECK:   n3 = $root.f(null, n2)
// CHECK:   ret null
// CHECK: }
function check_shape(): void {
  f(shape('a' => a(), 'b' => b()));
}

// TEST-CHECK-BAL: define $root.check_closure
// CHECK: define $root.check_closure($this: *void, $x: *HackInt) : *void {
// CHECK: local $impl: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$x
// CHECK:   n1 = __sil_allocate(<Closure$check_closure>)
// CHECK:   n2 = Closure$check_closure.__construct(n1, n0)
// CHECK:   store &$impl <- n1: *HackMixed
// CHECK:   ret null
// CHECK: }
function check_closure(int $x): void {
  $impl = () ==> { return $x; };
}

// TEST-CHECK-BAL: define $root.add_elem
// CHECK: define $root.add_elem($this: *void, $s1: *HackString, $s2: *HackString) : *void {
// CHECK: local $c: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_dict()
// CHECK:   n1: *HackMixed = load &$s1
// CHECK:   n2 = $builtins.hhbc_add_elem_c(n0, n1, $builtins.hack_int(0))
// CHECK:   n3: *HackMixed = load &$s2
// CHECK:   n4 = $builtins.hhbc_add_elem_c(n2, n3, $builtins.hack_int(1))
// CHECK:   store &$c <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }
function add_elem(string $s1, string $s2) : void {
  $c = dict [ $s1 => 0, $s2 => 1 ];
}

// TEST-CHECK-BAL: define $root.col_from_array
// CHECK: define $root.col_from_array($this: *void, $s1: *HackString, $s2: *HackString) : *void {
// CHECK: local $c1: *void, $c2: *void, $c3: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_dict()
// CHECK:   n1: *HackMixed = load &$s1
// CHECK:   n2 = $builtins.hhbc_add_elem_c(n0, n1, n1)
// CHECK:   n3: *HackMixed = load &$s2
// CHECK:   n4 = $builtins.hhbc_add_elem_c(n2, n3, n3)
// CHECK:   n5 = $builtins.hhbc_col_from_array_imm_set(n4)
// CHECK:   store &$c1 <- n5: *HackMixed
// CHECK:   n6 = $builtins.hhbc_new_dict()
// CHECK:   n7 = $builtins.hhbc_add_elem_c(n6, n1, $builtins.hack_int(1))
// CHECK:   n8 = $builtins.hhbc_add_elem_c(n7, n3, $builtins.hack_int(2))
// CHECK:   n9 = $builtins.hhbc_col_from_array_imm_map(n8)
// CHECK:   store &$c2 <- n9: *HackMixed
// CHECK:   n10 = $builtins.hhbc_new_vec(n1, n3)
// CHECK:   n11 = $builtins.hhbc_col_from_array_imm_vector(n10)
// CHECK:   store &$c3 <- n11: *HackMixed
// CHECK:   ret null
// CHECK: }
function col_from_array(string $s1, string $s2) : void {
  $c1 = ImmSet { $s1, $s2 };
  $c2 = ImmMap { $s1 => 1, $s2 => 2 };
  $c3 = ImmVector { $s1, $s2 };
}

// TEST-CHECK-BAL: define $root.check_switch
// CHECK: define $root.check_switch($this: *void, $x: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$x
// CHECK:   n1 = $builtins.hhbc_cmp_eq(n0, $builtins.hack_int(5))
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK:   n2 = $builtins.hhbc_print($builtins.hack_string("5"))
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK:   n3 = $builtins.hhbc_throw_non_exhaustive_switch()
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   ret null
// CHECK: }
function check_switch(int $x): void {
  switch ($x) {
    case 5: echo "5"; break;
  }
}

// TEST-CHECK-1: declare $builtins.hhbc_add_elem_c
// CHECK: declare $builtins.hhbc_add_elem_c(...): *HackMixed
