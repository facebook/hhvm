// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

class D {
  public int $foo = 42;
  /* HH_FIXME[4055] No initial value */
  public static D $bar;

  // TEST-CHECK-BAL: define D.mop_baseh_querym_pt
  // CHECK: define D.mop_baseh_querym_pt($this: .notnull *D) : .notnull *HackInt {
  // CHECK: #b0:
  // CHECK: // .column 12
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK: // .column 12
  // CHECK:   n1 = $builtins.hhbc_check_this(n0)
  // CHECK: // .column 12
  // CHECK:   n2: *HackMixed = load &$this
  // CHECK:   n3: *HackMixed = load n2.?.foo
  // CHECK: // .column 5
  // CHECK:   n4 = $builtins.hhbc_is_type_int(n3)
  // CHECK: // .column 5
  // CHECK:   n5 = $builtins.hhbc_verify_type_pred(n3, n4)
  // CHECK: // .column 5
  // CHECK:   ret n3
  // CHECK: }
  public function mop_baseh_querym_pt(): int {
    return $this->foo;
  }

  // TEST-CHECK-BAL: define D.mop_basesc_querym_pt
  // CHECK: define D.mop_basesc_querym_pt($this: .notnull *D) : .notnull *HackInt {
  // CHECK: #b0:
  // CHECK: // .column 15
  // CHECK:   n0 = __sil_lazy_class_initialize(<D>)
  // CHECK: // .column 12
  // CHECK:   n1: *HackMixed = load n0.?.bar
  // CHECK:   n2: *HackMixed = load n1.?.foo
  // CHECK: // .column 5
  // CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
  // CHECK: // .column 5
  // CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
  // CHECK: // .column 5
  // CHECK:   ret n2
  // CHECK: }
  public function mop_basesc_querym_pt(): int {
    return D::$bar->foo;
  }
}

// TEST-CHECK-BAL: define $root.mop_basec_querym_pc
// CHECK: define $root.mop_basec_querym_pc($this: *void) : .notnull *HackInt {
// CHECK: #b0:
// CHECK: // .column 10
// CHECK:   n0 = $root.ret_c(null)
// CHECK: // .column 10
// CHECK:   n1: *HackMixed = load n0.?.foo
// CHECK: // .column 3
// CHECK:   n2 = $builtins.hhbc_is_type_int(n1)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_verify_type_pred(n1, n2)
// CHECK: // .column 3
// CHECK:   ret n1
// CHECK: }
function mop_basec_querym_pc(): int {
  return ret_c()->foo;
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_ei
// CHECK: define $root.mop_basel_querym_ei($this: *void, $a: .notnull *HackVec) : .notnull *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 10
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK: // .column 3
// CHECK:   n6 = $builtins.hhbc_is_type_int(n5)
// CHECK: // .column 3
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK: // .column 3
// CHECK:   ret n5
// CHECK: }
function mop_basel_querym_ei(vec<int> $a): int {
  return $a[5];
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_ei_isset
// CHECK: define $root.mop_basel_querym_ei_isset($this: *void, $a: .notnull *HackVec) : .notnull *HackBool {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 16
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get_quiet(n4, n3)
// CHECK:   n6 = $builtins.hhbc_is_type_null(n5)
// CHECK: // .column 3
// CHECK:   n7 = $builtins.hhbc_is_type_bool(n6)
// CHECK: // .column 3
// CHECK:   n8 = $builtins.hhbc_verify_type_pred(n6, n7)
// CHECK: // .column 3
// CHECK:   ret n6
// CHECK: }
function mop_basel_querym_ei_isset(vec<int> $a): bool {
  return isset($a[5]);
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_pc
// CHECK: define $root.mop_basel_querym_pc($this: *void, $a: *C) : .notnull *HackInt {
// CHECK: #b0:
// CHECK: // .column 15
// CHECK:   n0 = $root.ret_str(null)
// CHECK: // .column 10
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hack_prop_get(n1, n0, false)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK: // .column 3
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK: // .column 3
// CHECK:   ret n2
// CHECK: }
function mop_basel_querym_pc(C $a): int {
  return $a->{ret_str()};
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_pl
// CHECK: define $root.mop_basel_querym_pl($this: *void, $a: *C) : .notnull *HackInt {
// CHECK: local $b: *void
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   store &$b <- $builtins.hack_string("hello"): *HackMixed
// CHECK: // .column 10
// CHECK:   n0: *HackMixed = load &$b
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hack_prop_get(n1, n0, false)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK: // .column 3
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK: // .column 3
// CHECK:   ret n2
// CHECK: }
function mop_basel_querym_pl(C $a): int {
  $b = "hello";
  /* HH_FIXME[2011] dynamic access */
  return $a->{$b};
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_el
// CHECK: define $root.mop_basel_querym_el($this: *void, $a: .notnull *HackVec, $b: .notnull *HackInt) : .notnull *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 10
// CHECK:   n3: *HackMixed = load &$b
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK: // .column 3
// CHECK:   n6 = $builtins.hhbc_is_type_int(n5)
// CHECK: // .column 3
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK: // .column 3
// CHECK:   ret n5
// CHECK: }
function mop_basel_querym_el(vec<int> $a, int $b): int {
  return $a[$b];
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_et
// CHECK: define $root.mop_basel_querym_et($this: *void, $a: .notnull *HackDict) : .notnull *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(4)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 10
// CHECK:   n3 = $builtins.hack_string("hello")
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK: // .column 3
// CHECK:   n6 = $builtins.hhbc_is_type_int(n5)
// CHECK: // .column 3
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK: // .column 3
// CHECK:   ret n5
// CHECK: }
function mop_basel_querym_et(dict<string, int> $a): int {
  return $a["hello"];
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_pt
// CHECK: define $root.mop_basel_querym_pt($this: *void, $a: *C) : .notnull *HackInt {
// CHECK: #b0:
// CHECK: // .column 10
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1: *HackMixed = load n0.?.foo
// CHECK: // .column 3
// CHECK:   n2 = $builtins.hhbc_is_type_int(n1)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_verify_type_pred(n1, n2)
// CHECK: // .column 3
// CHECK:   ret n1
// CHECK: }
function mop_basel_querym_pt(C $a): int {
  return $a->foo;
}

// TEST-CHECK-BAL: define $root.mop_basel_querym_qt
// CHECK: define $root.mop_basel_querym_qt($this: *void, $a: *C) : *HackInt {
// CHECK: #b0:
// CHECK: // .column 10
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $builtins.hack_prop_get(n0, "foo", true)
// CHECK: // .column 3
// CHECK:   n2 = $builtins.hhbc_is_type_null(n1)
// CHECK: // .column 3
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 3
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK: // .column 3
// CHECK:   jmp b3($builtins.hack_bool(true))
// CHECK: #b2:
// CHECK: // .column 3
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_is_type_int(n1)
// CHECK: // .column 3
// CHECK:   jmp b3(n3)
// CHECK: #b3(n4: *HackMixed):
// CHECK: // .column 3
// CHECK:   n5 = $builtins.hhbc_verify_type_pred(n1, n4)
// CHECK: // .column 3
// CHECK:   ret n1
// CHECK: }
function mop_basel_querym_qt(?C $a): ?int {
  return $a?->foo;
}

// TEST-CHECK-BAL: define $root.mop_basel_setm_w
// CHECK: define $root.mop_basel_setm_w($this: *void, $a: .notnull *HackVec) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_cow_append(n4, n3)
// CHECK:   store &$a <- n5: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function mop_basel_setm_w(vec<int> $a): void {
  $a[] = 5;
}

// TEST-CHECK-BAL: define $root.mop_basel_incdec_ei
// CHECK: define $root.mop_basel_incdec_ei($this: *void, $a: .notnull *HackVec) : .notnull *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 10
// CHECK:   n3 = $builtins.hack_int(3)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK:   n6 = $builtins.hhbc_add(n5, $builtins.hack_int(1))
// CHECK:   n7: *HackMixed = load &$a
// CHECK:   n8 = $builtins.hack_array_cow_set(n7, n3, n6)
// CHECK:   store &$a <- n8: *HackMixed
// CHECK: // .column 3
// CHECK:   n9 = $builtins.hhbc_is_type_int(n5)
// CHECK: // .column 3
// CHECK:   n10 = $builtins.hhbc_verify_type_pred(n5, n9)
// CHECK: // .column 3
// CHECK:   ret n5
// CHECK: }
function mop_basel_incdec_ei(vec<int> $a): int {
  /* HH_FIXME[1002] Assignment as expression */
  return $a[3]++;
}

// TEST-CHECK-BAL: define $root.mop_basel_unset_ei
// CHECK: define $root.mop_basel_unset_ei($this: *void, $a: .notnull *HackDict) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 9
// CHECK:   n3 = $builtins.hack_int(5)
// CHECK:   n4: *HackMixed = load &$a
// CHECK:   n5 = $builtins.hack_array_cow_unset(n4, n3)
// CHECK:   store &$a <- n5: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function mop_basel_unset_ei(dict<int, int> $a): void {
  unset($a[5]);
}

// TEST-CHECK-BAL: define $root.mop_basel_unset_pt
// CHECK: define $root.mop_basel_unset_pt($this: *void, $a: *C) : *void {
// CHECK: #b0:
// CHECK: // .column 1
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   store n0.?.foo <- null: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function mop_basel_unset_pt(C $a): void {
  /* HH_FIXME[4135] Allow unset */
  unset($a->foo);
}

// TEST-CHECK-BAL: define $root.mop_basec_querym_cget(
// CHECK: define $root.mop_basec_querym_cget($this: *void, $d: .notnull *HackDict) : .notnull *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(4)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$d
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 10
// CHECK:   n3 = $builtins.hack_string("k")
// CHECK:   n4: *HackMixed = load &$d
// CHECK:   n5 = $builtins.hack_array_get(n4, n3)
// CHECK: // .column 3
// CHECK:   n6 = $builtins.hhbc_is_type_int(n5)
// CHECK: // .column 3
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK: // .column 3
// CHECK:   ret n5
// CHECK: }
function mop_basec_querym_cget(dict<string, int> $d): int {
  return $d['k'];
}

// TEST-CHECK-BAL: define $root.mop_basec_querym_cgetquiet(
// CHECK: define $root.mop_basec_querym_cgetquiet($this: *void, $d: .notnull *HackDict) : .notnull *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(4)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$d
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 10
// CHECK:   n3 = $builtins.hack_string("k")
// CHECK:   n4: *HackMixed = load &$d
// CHECK:   n5 = $builtins.hack_array_get_quiet(n4, n3)
// CHECK: // .column 10
// CHECK:   n6 = $builtins.hhbc_is_type_null(n5)
// CHECK: // .column 10
// CHECK:   n7 = $builtins.hhbc_not(n6)
// CHECK: // .column 10
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 10
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK: // .column 10
// CHECK:   jmp b3(n5)
// CHECK: #b2:
// CHECK: // .column 10
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK: // .column 21
// CHECK:   jmp b3($builtins.hack_int(42))
// CHECK: #b3(n8: *HackMixed):
// CHECK: // .column 3
// CHECK:   n9 = $builtins.hhbc_is_type_int(n8)
// CHECK: // .column 3
// CHECK:   n10 = $builtins.hhbc_verify_type_pred(n8, n9)
// CHECK: // .column 3
// CHECK:   ret n8
// CHECK: }
function mop_basec_querym_cgetquiet(dict<string, int> $d): int {
  return $d['k'] ?? 42;
}
