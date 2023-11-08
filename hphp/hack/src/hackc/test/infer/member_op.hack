// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

class D {
  public int $foo = 42;
  /* HH_FIXME[4055] No initial value */
  public static D $bar;

  // TEST-CHECK-BAL: define D.mop_baseh_querym_pt
  // CHECK: define D.mop_baseh_querym_pt($this: *D) : *HackInt {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK:   n1 = $builtins.hhbc_check_this(n0)
  // CHECK:   n2: *HackMixed = load &$this
  // CHECK:   n3: *HackMixed = load n2.?.foo
  // CHECK:   n4 = $builtins.hhbc_is_type_int(n3)
  // CHECK:   n5 = $builtins.hhbc_verify_type_pred(n3, n4)
  // CHECK:   ret n3
  // CHECK: }
  public function mop_baseh_querym_pt(): int {
    return $this->foo;
  }

  // TEST-CHECK-BAL: define D.mop_basesc_querym_pt
  // CHECK: define D.mop_basesc_querym_pt($this: *D) : *HackInt {
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hhbc_class_get_c($builtins.hack_string("D"))
  // CHECK:   n1: *HackMixed = load n0.?.bar
  // CHECK:   n2: *HackMixed = load n1.?.foo
  // CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
  // CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
  // CHECK:   ret n2
  // CHECK: }
  public function mop_basesc_querym_pt(): int {
    return D::$bar->foo;
  }
}

// TEST-CHECK-BAL: define $root.mop_basec_querym_pc
// CHECK: define $root.mop_basec_querym_pc($this: *void) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $root.ret_c(null)
// CHECK:   n1: *HackMixed = load n0.?.foo
// CHECK:   n2 = $builtins.hhbc_is_type_int(n1)
// CHECK:   n3 = $builtins.hhbc_verify_type_pred(n1, n2)
// CHECK:   ret n1
// CHECK: }
function mop_basec_querym_pc(): int {
  return ret_c()->foo;
}

// TEST-CHECK-BAL: define $root.mop_basegc_querym_ec
// CHECK: define $root.mop_basegc_querym_ec($this: *void) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $root.ret_int(null)
// CHECK:   n1 = $builtins.hack_get_superglobal($builtins.hack_string("_SERVER"))
// CHECK:   n2 = $builtins.hack_array_get(n1, n0)
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK:   ret n2
// CHECK: }
function mop_basegc_querym_ec(): int {
  /* HH_FIXME[2050] Hack doesn't know about $_SERVER */
  return $_SERVER[ret_int()];
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_ei
// CHECK: define $root.mop_basel_querym_ei($this: *void, $a: *HackVec) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK:   n6 = $builtins.hhbc_is_type_int(n5)
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK:   ret n5
// CHECK: }
function mop_basel_querym_ei(vec<int> $a): int {
  return $a[5];
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_ei_isset
// CHECK: define $root.mop_basel_querym_ei_isset($this: *void, $a: *HackVec) : *HackBool {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK:   n6 = $builtins.hhbc_is_type_null(n5)
// CHECK:   n7 = $builtins.hhbc_is_type_bool(n6)
// CHECK:   n8 = $builtins.hhbc_verify_type_pred(n6, n7)
// CHECK:   ret n6
// CHECK: }
function mop_basel_querym_ei_isset(vec<int> $a): bool {
  return isset($a[5]);
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_pc
// CHECK: define $root.mop_basel_querym_pc($this: *void, $a: *C) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $root.ret_str(null)
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hack_prop_get(n1, n0, false)
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK:   ret n2
// CHECK: }
function mop_basel_querym_pc(C $a): int {
  return $a->{ret_str()};
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_pl
// CHECK: define $root.mop_basel_querym_pl($this: *void, $a: *C) : *HackInt {
// CHECK: local $b: *void
// CHECK: #b0:
// CHECK:   store &$b <- $builtins.hack_string("hello"): *HackMixed
// CHECK:   n0: *HackMixed = load &$b
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hack_prop_get(n1, n0, false)
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK:   ret n2
// CHECK: }
function mop_basel_querym_pl(C $a): int {
  $b = "hello";
  /* HH_FIXME[2011] dynamic access */
  return $a->{$b};
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_el
// CHECK: define $root.mop_basel_querym_el($this: *void, $a: *HackVec, $b: *HackInt) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3: *HackMixed = load &$b
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK:   n6 = $builtins.hhbc_is_type_int(n5)
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK:   ret n5
// CHECK: }
function mop_basel_querym_el(vec<int> $a, int $b): int {
  return $a[$b];
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_et
// CHECK: define $root.mop_basel_querym_et($this: *void, $a: *HackDict) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(4)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hack_string("hello")
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK:   n6 = $builtins.hhbc_is_type_int(n5)
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK:   ret n5
// CHECK: }
function mop_basel_querym_et(dict<string, int> $a): int {
  return $a["hello"];
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_pt
// CHECK: define $root.mop_basel_querym_pt($this: *void, $a: *C) : *HackInt {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1: *HackMixed = load n0.?.foo
// CHECK:   n2 = $builtins.hhbc_is_type_int(n1)
// CHECK:   n3 = $builtins.hhbc_verify_type_pred(n1, n2)
// CHECK:   ret n1
// CHECK: }
function mop_basel_querym_pt(C $a): int {
  return $a->foo;
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_qt
// CHECK: define $root.mop_basel_querym_qt($this: *void, $a: *C) : *HackInt {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $builtins.hack_prop_get(n0, "foo", true)
// CHECK:   n2 = $builtins.hhbc_is_type_null(n1)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK:   jmp b3($builtins.hack_bool(true))
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK:   n3 = $builtins.hhbc_is_type_int(n1)
// CHECK:   jmp b3(n3)
// CHECK: #b3(n4: *HackMixed):
// CHECK:   n5 = $builtins.hhbc_verify_type_pred(n1, n4)
// CHECK:   ret n1
// CHECK: }
function mop_basel_querym_qt(?C $a): ?int {
  return $a?->foo;
}

// TEST-CHECK-BAL: define $root.mop_basel_setm_w
// CHECK: define $root.mop_basel_setm_w($this: *void, $a: *HackVec) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_cow_append(n4, n3)
// CHECK:   store &$a <- n5: *HackMixed
// CHECK:   ret null
// CHECK: }
function mop_basel_setm_w(vec<int> $a): void {
  $a[] = 5;
}

// TEST-CHECK-BAL: define $root.mop_basel_incdec_ei
// CHECK: define $root.mop_basel_incdec_ei($this: *void, $a: *HackVec) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hack_int(3)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK:   n6 = $builtins.hhbc_add(n5, $builtins.hack_int(1))
// CHECK:   n7: *HackMixed = load &$a
// CHECK:   n8 = $builtins.hack_array_cow_set(n7, n3, n6)
// CHECK:   store &$a <- n8: *HackMixed
// CHECK:   n9 = $builtins.hhbc_is_type_int(n5)
// CHECK:   n10 = $builtins.hhbc_verify_type_pred(n5, n9)
// CHECK:   ret n5
// CHECK: }
function mop_basel_incdec_ei(vec<int> $a): int {
  /* HH_FIXME[1002] Assignment as expression */
  return $a[3]++;
}

// TEST-CHECK-BAL: define $root.mop_basel_unset_ei
// CHECK: define $root.mop_basel_unset_ei($this: *void, $a: *HackDict) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_cow_unset(n4, n3)
// CHECK:   store &$a <- n5: *HackMixed
// CHECK:   ret null
// CHECK: }
function mop_basel_unset_ei(dict<int, int> $a): void {
  unset($a[5]);
}

// TEST-CHECK-BAL: define $root.mop_basel_unset_pt
// CHECK: define $root.mop_basel_unset_pt($this: *void, $a: *C) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   store n0.?.foo <- null: *HackMixed
// CHECK:   ret null
// CHECK: }
function mop_basel_unset_pt(C $a): void {
  /* HH_FIXME[4135] Allow unset */
  unset($a->foo);
}
