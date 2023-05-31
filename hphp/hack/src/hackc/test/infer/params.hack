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
// CHECK:   n1 = __sil_lazy_class_initialize(<Internal>)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2 = __sil_allocate(<Internal>)
// CHECK:   n3 = Internal._86pinit(n2)
// CHECK:   store &$2 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5 = $builtins.hhbc_class_has_reified_generics(n4)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n5)
// CHECK:   n6: *HackMixed = load &$0
// CHECK:   n7 = $builtins.hhbc_has_reified_parent(n6)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n8: *HackMixed = load &$2
// CHECK:   n9 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n10 = n8.?._86reifiedinit(n9)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n5)
// CHECK:   n11: *HackMixed = load &$0
// CHECK:   n12 = $builtins.hhbc_class_get_c(n11)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n13: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n14: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n15 = $builtins.hhbc_throw(n14)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n16 = n13.?.__construct()
// CHECK:   n17 = $builtins.hhbc_lock_obj(n13)
// CHECK:   n18 = $builtins.hhbc_is_type_struct_c(n13, n0, $builtins.hack_int(1))
// CHECK:   n19 = $builtins.hhbc_verify_type_pred(n13, n18)
// CHECK:   ret n13
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
// CHECK:   n1 = __sil_lazy_class_initialize(<External>)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2 = __sil_allocate(<External>)
// CHECK:   n3 = External._86pinit(n2)
// CHECK:   store &$2 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5 = $builtins.hhbc_class_has_reified_generics(n4)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n5)
// CHECK:   n6: *HackMixed = load &$0
// CHECK:   n7 = $builtins.hhbc_has_reified_parent(n6)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n8: *HackMixed = load &$2
// CHECK:   n9 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n10 = n8.?._86reifiedinit(n9)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n5)
// CHECK:   n11: *HackMixed = load &$0
// CHECK:   n12 = $builtins.hhbc_class_get_c(n11)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n13: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n14: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n15 = $builtins.hhbc_throw(n14)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n16 = n13.?.__construct()
// CHECK:   n17 = $builtins.hhbc_lock_obj(n13)
// CHECK:   n18 = $builtins.hhbc_is_type_struct_c(n13, n0, $builtins.hack_int(1))
// CHECK:   n19 = $builtins.hhbc_verify_type_pred(n13, n18)
// CHECK:   ret n13
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
