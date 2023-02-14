// RUN: %hackc compile-infer --keep-going %s | FileCheck %s

class Internal {}

class InternalGeneric<T> {}

// TEST-CHECK-BAL: define $root.internalClassParam
// CHECK: define $root.internalClassParam($this: *void, $a: *HackInt, $b: *Internal) : *Internal {
// CHECK: local $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(101), $builtins.hack_string("classname"), $builtins.hack_string("Internal"))
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1: *Internal$static = load &Internal$static::static_singleton
// CHECK:   n2 = $builtins.lazy_initialize(n1)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n3: *Internal$static = load &Internal$static::static_singleton
// CHECK:   n4 = $builtins.lazy_initialize(n3)
// CHECK:   n5 = n3.HackMixed.__factory()
// CHECK:   store &$2 <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$0
// CHECK:   n7 = $builtins.hhbc_class_has_reified_generics(n6)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   n8: *HackMixed = load &$0
// CHECK:   n9 = $builtins.hhbc_has_reified_parent(n8)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n9)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n9)
// CHECK:   n10: *HackMixed = load &$2
// CHECK:   n11 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n12 = n10.HackMixed._86reifiedinit(n11)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n13: *HackMixed = load &$0
// CHECK:   n14 = $builtins.hhbc_class_get_c(n13)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n15: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n16: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n17 = $builtins.hhbc_throw(n16)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n18 = n15.HackMixed.__construct()
// CHECK:   n19 = $builtins.hhbc_lock_obj(n15)
// CHECK:   n20 = $builtins.hhbc_is_type_struct_c(n15, n0, $builtins.hack_int(1))
// CHECK:   n21 = $builtins.hhbc_verify_type_pred(n15, n20)
// CHECK:   ret n15
// CHECK: }
function internalClassParam(int $a, Internal $b) : Internal {
  return new Internal();
}

// TEST-CHECK-BAL: define $root.externalClassParam
// CHECK: define $root.externalClassParam($this: *void, $a: *HackBool, $b: *External) : *External {
// CHECK: local $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(101), $builtins.hack_string("classname"), $builtins.hack_string("External"))
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1: *External$static = load &External$static::static_singleton
// CHECK:   n2 = $builtins.lazy_initialize(n1)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n3: *External$static = load &External$static::static_singleton
// CHECK:   n4 = $builtins.lazy_initialize(n3)
// CHECK:   n5 = n3.HackMixed.__factory()
// CHECK:   store &$2 <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$0
// CHECK:   n7 = $builtins.hhbc_class_has_reified_generics(n6)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   n8: *HackMixed = load &$0
// CHECK:   n9 = $builtins.hhbc_has_reified_parent(n8)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n9)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n9)
// CHECK:   n10: *HackMixed = load &$2
// CHECK:   n11 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n12 = n10.HackMixed._86reifiedinit(n11)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n13: *HackMixed = load &$0
// CHECK:   n14 = $builtins.hhbc_class_get_c(n13)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n15: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n16: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n17 = $builtins.hhbc_throw(n16)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n18 = n15.HackMixed.__construct()
// CHECK:   n19 = $builtins.hhbc_lock_obj(n15)
// CHECK:   n20 = $builtins.hhbc_is_type_struct_c(n15, n0, $builtins.hack_int(1))
// CHECK:   n21 = $builtins.hhbc_verify_type_pred(n15, n20)
// CHECK:   ret n15
// CHECK: }
function externalClassParam(bool $a, External $b): External {
  return new External();
}

// TEST-CHECK-BAL: define $root.genericParams
// CHECK: define $root.genericParams($this: *void, $a: *HackString, $b: *InternalGeneric) : *HackInt {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(101), $builtins.hack_string("classname"), $builtins.hack_string("InternalGeneric"), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(4))))
// CHECK:   n1: *HackMixed = load &$b
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hhbc_is_type_int($builtins.hack_int(42))
// CHECK:   n4 = $builtins.hhbc_verify_type_pred($builtins.hack_int(42), n3)
// CHECK:   ret $builtins.hack_int(42)
// CHECK: }
async function genericParams(string $a, InternalGeneric<string> $b): Awaitable<int> {
  return 42;
}
