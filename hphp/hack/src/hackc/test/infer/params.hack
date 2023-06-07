// RUN: %hackc compile-infer --keep-going %s | FileCheck %s

class Internal {}

class InternalGeneric<T> {}

// TEST-CHECK-BAL: define $root.internalClassParam
// CHECK: define $root.internalClassParam($this: *void, $a: *HackInt, $b: *Internal) : *Internal {
// CHECK: local $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n0 = __sil_lazy_class_initialize(<Internal>)
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK:   n1 = __sil_allocate(<Internal>)
// CHECK:   n2 = Internal._86pinit(n1)
// CHECK:   store &$2 <- n1: *HackMixed
// CHECK:   n3: *HackMixed = load &$0
// CHECK:   n4 = $builtins.hhbc_class_has_reified_generics(n3)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   n5: *HackMixed = load &$0
// CHECK:   n6 = $builtins.hhbc_has_reified_parent(n5)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK:   n7: *HackMixed = load &$2
// CHECK:   n8 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n9 = n7.?._86reifiedinit(n8)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   n10: *HackMixed = load &$0
// CHECK:   n11 = $builtins.hhbc_class_get_c(n10)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n12: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n13: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n14 = $builtins.hhbc_throw(n13)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n15 = n12.?.__construct()
// CHECK:   n16 = $builtins.hhbc_lock_obj(n12)
// CHECK:   n17 = $builtins.hack_bool(__sil_instanceof(n12, <Internal>))
// CHECK:   n18 = $builtins.hhbc_verify_type_pred(n12, n17)
// CHECK:   ret n12
// CHECK: }
function internalClassParam(int $a, Internal $b) : Internal {
  return new Internal();
}

// TEST-CHECK-BAL: define $root.externalClassParam
// CHECK: define $root.externalClassParam($this: *void, $a: *HackBool, $b: *External) : *External {
// CHECK: local $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n0 = __sil_lazy_class_initialize(<External>)
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK:   n1 = __sil_allocate(<External>)
// CHECK:   n2 = External._86pinit(n1)
// CHECK:   store &$2 <- n1: *HackMixed
// CHECK:   n3: *HackMixed = load &$0
// CHECK:   n4 = $builtins.hhbc_class_has_reified_generics(n3)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   n5: *HackMixed = load &$0
// CHECK:   n6 = $builtins.hhbc_has_reified_parent(n5)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK:   n7: *HackMixed = load &$2
// CHECK:   n8 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n9 = n7.?._86reifiedinit(n8)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   n10: *HackMixed = load &$0
// CHECK:   n11 = $builtins.hhbc_class_get_c(n10)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n12: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n13: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n14 = $builtins.hhbc_throw(n13)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n15 = n12.?.__construct()
// CHECK:   n16 = $builtins.hhbc_lock_obj(n12)
// CHECK:   n17 = $builtins.hack_bool(__sil_instanceof(n12, <External>))
// CHECK:   n18 = $builtins.hhbc_verify_type_pred(n12, n17)
// CHECK:   ret n12
// CHECK: }
function externalClassParam(bool $a, External $b): External {
  return new External();
}

// TEST-CHECK-BAL: define .async $root.genericParams
// CHECK: define .async $root.genericParams($this: *void, $a: *HackString, $b: *InternalGeneric) : *HackInt {
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
